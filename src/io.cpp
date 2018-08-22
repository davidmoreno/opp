#include <iostream>
#include <unistd.h>
#include "io.hpp"
#include "opp.hpp"

namespace opp::io{
  static opp::symbol PRINT("print");
  static opp::symbol READLINE("readline");
  static opp::symbol READLINE_RESULT("readline_result");

  file *stdin = nullptr;
  file *stdout = nullptr;
  file *stderr = nullptr;

  file::file(std::string &&name, int fd) : process(std::string(name)), filename(name), fd(fd){
  }

  file::~file(){
    close(fd);
  }

  void file::loop(){
    // printf("process file %s %d\n", filename.c_str(), fd);
    // Build once, use man times
    std::map<symbol, std::function<void(const std::any &)>> _case = {
      {PRINT, [this](const std::any &args){
        // printf("Resolved print %s %d\n", filename.c_str(), fd);
        auto str = std::any_cast<std::string>(args);
        auto wrote = write(this->fd, str.c_str(), str.size());
        if ((wrote < 0) || (unsigned(wrote) < str.size())){
          throw write_error();
        }
      }},
      {READLINE, [this](const std::any &args){
        auto from = std::any_cast<process*>(args);
        // printf("%s: Answer for %s\n", name().c_str(), from->name().c_str());
        std::string ret="";
        char c;
        do{
          ssize_t n = read(fd, &c, 1);
          if (n!=1){
            fprintf(::stderr, "stdin closed\n");
            from->send(EXIT, this);
            throw opp::io::read_error();
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

  void file::print_(std::string &&str){
    send(PRINT, str);
  }

  std::string file::readline(){
    send(READLINE, {opp::self()});
    auto res = opp::self()->receive(READLINE_RESULT, process::FOREVER);
    std::string str = std::any_cast<std::string>(res);
    return str;
  }
}
