#include <iostream>
#include <unistd.h>
#include "io.hpp"
#include "opp.hpp"

namespace opp::IO{
  static opp::Symbol PRINT("print");
  static opp::Symbol PRINTLN("println");
  static opp::Symbol READLINE("readline");
  static opp::Symbol READLINE_RESULT("readline_result");

  File *stdin = nullptr;
  File *stdout = nullptr;
  File *stderr = nullptr;

  File::File(std::string &&name, int fd) : Process(std::string(name)), filename(name), fd(fd){
  }

  File::~File(){
    close(fd);
  }

  void File::loop(){
    // printf("Process file %s %d\n", filename.c_str(), fd);
    // Build once, use man times
    std::map<Symbol, std::function<void(const std::any &)>> _case = {
      {PRINT, [this](const std::any &args){
        // printf("Resolved print %s %d\n", filename.c_str(), fd);
        auto str = std::any_cast<std::string>(args);
        auto wrote = write(this->fd, str.c_str(), str.size());
        if ((wrote < 0) || (unsigned(wrote) < str.size())){
          throw write_error();
        }
      }},
      {PRINTLN, [this](const std::any &args){
        // printf("Resolved print %s %d\n", filename.c_str(), fd);
        auto str = std::any_cast<std::string>(args);
        auto wrote = write(this->fd, str.c_str(), str.size());
        if (wrote != signed(str.size())){
          throw write_error();
        }
        wrote = write(this->fd, "\n", 1);
        if (wrote != 1){
          throw write_error();
        }
      }},
      {READLINE, [this](const std::any &args){
        auto from = std::any_cast<Process*>(args);
        // printf("%s: Answer for %s\n", name().c_str(), from->name().c_str());
        std::string ret="";
        char c;
        do{
          ssize_t n = read(fd, &c, 1);
          if (n!=1){
            from->send(EXIT, this);
            throw opp::IO::read_error();
          }
          ret+=c;
        }while(c!='\n');
        from->send(READLINE_RESULT, ret);
      }}
    };

    while(true){ // This will exit because of an exception when closed
      receive(_case, FOREVER);
    }
  }

  void File::print(const std::string &str){
    send(PRINT, str);
  }

  void File::println(const std::string &str){
    send(PRINTLN, str);
  }

  std::string File::readline(){
    send(READLINE, {opp::self()});
    auto res = opp::self()->receive(READLINE_RESULT, Process::FOREVER);
    std::string str = std::any_cast<std::string>(res);
    return str;
  }
}
