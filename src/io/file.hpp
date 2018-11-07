#pragma once

#include <fmt/format.h>
#include "core/process.hpp"
#include "core/exceptions.hpp"
#include "core/string.hpp"
#include "buffer.hpp"

namespace opp::io{
  typedef enum{
    read = 1,
    write = 2,
    readwrite = 3,
  }  open_mode_e;

  class file : public opp::process{
    std::string filename;
    int fd;
  public:
    file(std::string filename, int fd);
    file(std::string filename,  open_mode_e om);
    ~file();
    void replace_fd(int fd);
    int get_fd(){ return fd; };

    virtual void loop();
    virtual void stop();
    virtual void close();

    // io funcs
    std::string readline();
    void write(const std::string &data);
    void write(buffer_t &data);
    void read(buffer_t &data);
    buffer_t read();
    bool eof();

    void print_(std::string str);
    ///////// helpers

    // out funcs
    template<typename... Args>
    void print(Args ...args){
      print_(fmt::format(args...));
    }
    template<typename... Args>
    void println(Args... args){
      print_(fmt::format(args...) + "\n");
    }
  };

  extern std::shared_ptr<file> stdin;
  extern std::shared_ptr<file> stdout;
  extern std::shared_ptr<file> stderr;
};
