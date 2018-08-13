#include <unistd.h>
#include "io.hpp"

namespace popc::IO{
  static popc::Symbol PRINT;
  static popc::Symbol READLINE;
  static popc::Symbol READLINE_RESULT;

  File::File(std::string &&name, int fd) : filename(name), fd(fd){
  }

  File::~File(){
    close(fd);
  }

  void File::process(){
    // Build once, use man times
    std::map<Symbol, std::function<void(const std::any &)>> _case = {
      {PRINT, [this](const std::any &args){
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
