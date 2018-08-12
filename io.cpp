#include <unistd.h>
#include "io.hpp"

namespace SPC::IO{
  static SPC::Symbol PRINT;
  static SPC::Symbol READLINE;
  static SPC::Symbol READLINE_RESULT;


  File::File(std::string &&name, int fd) : filename(filename), fd(fd){
  }

  File::~File(){
    close(fd);
  }

  void File::process(){
    // Build once, use man times
    std::map<Symbol, std::function<void(const std::any &)>> _case = {
      {PRINT, [this](const std::any &args){
        auto str = std::any_cast<std::string>(args);
        write(this->fd, str.c_str(), str.size());
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
