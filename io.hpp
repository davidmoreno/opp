#pragma once

#include "process.hpp"

namespace opp::IO{
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

  class File : opp::Process{
    std::string filename;
    int fd;
  public:
    File(std::string &&filename, int fd);
    File(std::string &&filename);
    ~File();
    virtual void loop();


    // out funcs
    void print(const std::string &);
    void println(const std::string &);
    // in funcs
    std::string readline();
  };


  extern File *stdin;
  extern File *stdout;
  extern File *stderr;
};
