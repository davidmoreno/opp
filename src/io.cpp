#include <iostream>
#include <unistd.h>
#include "io.hpp"
#include "opp.hpp"

namespace opp::io{
  static opp::symbol PRINT("print");
  static opp::symbol READLINE("readline");
  static opp::symbol READLINE_RESULT("readline_result");

  struct msg_print{ std::string str; };
  struct msg_readline{ opp::process *from; };

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
    std::function<void(const msg_print &)> printfn = [this](const msg_print &msg){
      auto str = msg.str;
      auto wrote = write(this->fd, str.c_str(), str.size());
      if ((wrote < 0) || (unsigned(wrote) < str.size())){
        throw write_error();
      }
    };
    std::function<void(const msg_readline &)> readlinefn = [this](const msg_readline &msg){
      std::string ret="";
      char c;
      do{
        ssize_t n = read(fd, &c, 1);
        if (n!=1){
          fprintf(::stderr, "stdin closed\n");
          msg.from->send(EXIT, this);
          throw opp::io::read_error();
        }
        ret+=c;
      }while(c!='\n');
      msg.from->send(READLINE_RESULT, ret);
    };

    // std::map<symbol, std::function<void(const std::any &)>> _case = {
    //   {PRINT, [&printfn](const std::any &args){
    //     auto msg = std::any_cast<msg_print>(args);
    //     printfn(msg);;
    //   }},
    //   {READLINE, [&readlinefn](const std::any &args){
    //     auto msg = std::any_cast<msg_readline>(args);
    //     readlinefn(msg);
    //     // printf("%s: Answer for %s\n", name().c_str(), from->name().c_str());
    //   }}
    // };

    while(true){ // This will exit because of an exception when closed
      receive<msg_print, msg_readline>(printfn, readlinefn, FOREVER);
    }
  }

  void file::print_(std::string &&str){
    send(PRINT, msg_print{str});
  }

  std::string file::readline(){
    send(READLINE, msg_readline{opp::self()});
    auto res = opp::self()->receive(READLINE_RESULT, process::FOREVER);
    std::string str = std::any_cast<std::string>(res);
    return str;
  }
}
