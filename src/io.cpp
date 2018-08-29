#include <iostream>
#include <unistd.h>
#include "io.hpp"
#include "opp.hpp"

namespace opp::io{
  struct print_msg{ std::string str; };
  struct readline_msg{ std::shared_ptr<opp::process> from; };
  struct readline_result_msg{ std::string string; };

  std::shared_ptr<file> stdin;
  std::shared_ptr<file> stdout;
  std::shared_ptr<file> stderr;

  file::file(std::string &&name, int fd) : process(std::string(name)), filename(name), fd(fd){
  }

  file::~file(){
    close(fd);
  }

  void file::loop(){
    // printf("process file %s %d\n", filename.c_str(), fd);
    // Build once, use man times
    std::function<void(const print_msg &)> printfn = [this](const print_msg &msg){
      auto str = msg.str;
      auto wrote = write(this->fd, str.c_str(), str.size());
      if ((wrote < 0) || (unsigned(wrote) < str.size())){
        throw write_error();
      }
    };
    std::function<void(const readline_msg &)> readlinefn = [this](const readline_msg &msg){
      std::string ret="";
      char c;
      do{
        ssize_t n = read(fd, &c, 1);
        if (n!=1){
          fprintf(::stderr, "stdin closed\n");
          msg.from->send(exit_msg{shared_from_this()});
          throw opp::io::read_error();
        }
        ret+=c;
      }while(c!='\n');
      msg.from->send(readline_result_msg{ret});
    };

    // std::map<symbol, std::function<void(const std::any &)>> _case = {
    //   {PRINT, [&printfn](const std::any &args){
    //     auto msg = std::any_cast<print_msg>(args);
    //     printfn(msg);;
    //   }},
    //   {READLINE, [&readlinefn](const std::any &args){
    //     auto msg = std::any_cast<readline_msg>(args);
    //     readlinefn(msg);
    //     // printf("%s: Answer for %s\n", name().c_str(), from->name().c_str());
    //   }}
    // };

    while(true){ // This will exit because of an exception when closed
      receive<print_msg, readline_msg>(printfn, readlinefn, FOREVER);
    }
  }

  void file::print_(std::string &&str){
    send(print_msg{str});
  }

  std::string file::readline(){
    send(readline_msg{opp::self()});
    auto res = opp::self()->receive<readline_result_msg>(process::FOREVER);
    return res.string;
  }
}
