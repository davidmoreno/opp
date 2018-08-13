#pragma once

#include "process.hpp"

namespace popc::IO{
  class write_error : public std::exception{
    virtual const char* what() const throw(){
      return "write error";
    }
  };

  class File : popc::Process{
    std::string filename;
    int fd;
  public:
    File(std::string &&filename, int fd);
    File(std::string &&filename);
    ~File();
    virtual void process();


    // out funcs
    void print(const std::string &);
    void println(const std::string &);
    // in funcs
    std::string readline();
  };


  extern File stdin;
  extern File stdout;
  extern File stderr;
};
