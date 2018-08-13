#include <iostream>
#include <unistd.h>
#include "io.hpp"

namespace popc::IO{
  static popc::Symbol PRINT("print");
  static popc::Symbol READLINE("readline");
  static popc::Symbol READLINE_RESULT("readline_result");

  File stdin("stdin", 0);
  File stdout("stdout", 1);
  File stderr("stderr", 2);

  File::File(std::string &&name, int fd) : Process(std::string(name)), filename(name), fd(fd){
  }

  File::~File(){
    close(fd);
  }

  void File::process(){
    printf("Process file %s %d\n", filename.c_str(), fd);
    // Build once, use man times
    std::map<Symbol, std::function<void(const std::any &)>> _case = {
      {PRINT, [this](const std::any &args){
        printf("Resolved print %s %d\n", filename.c_str(), fd);
        auto str = std::any_cast<std::string>(args);
        auto wrote = write(this->fd, str.c_str(), str.size());
        if ((wrote < 0) || (unsigned(wrote) < str.size())){
          throw write_error();
        }
      }},
    };

    while(true){ // This will exit because of an exception when closed
      receive(_case);
    }
  }

  void File::print(const std::string &str){
    send(PRINT, str);
  }

  std::string File::readline(){
    send(READLINE, {this});
    return std::any_cast<std::string>(receive(READLINE_RESULT));
  }
}
