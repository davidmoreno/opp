#pragma once

#include "core/process.hpp"
#include "core/exceptions.hpp"
#include "core/string.hpp"

namespace opp::io{
  class write_error : public opp::exception{
    virtual const char* what() const throw(){
      return "write error";
    }
  };

  class read_error : public opp::exception{
    virtual const char* what() const throw(){
      return "read error";
    }
  };

  class file : public opp::process{
    std::string filename;
    int fd;
  public:
    file(std::string &&filename, int fd);
    file(std::string &&filename);
    ~file();
    virtual void loop();
    virtual void stop();

    // out funcs
    void print_(std::string &&str);
    template<typename... Args>
    void print(Args ...args){
      print_(concat(args...));
    }
    template<typename... Args>
    void println(Args... args){
      print_(concat(args..., '\n'));
    }
    // in funcs
    std::string readline();
  };


  extern std::shared_ptr<file> stdin;
  extern std::shared_ptr<file> stdout;
  extern std::shared_ptr<file> stderr;
};
