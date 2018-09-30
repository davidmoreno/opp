#pragma once

#include <fmt/format.h>
#include "core/process.hpp"
#include "core/exceptions.hpp"
#include "core/string.hpp"

namespace opp::io{
  class exception : public opp::exception{
  public:
    exception(std::string str="") : opp::exception(std::move(str)){}
  };

  class write_error : public exception{
  public:
    write_error() : exception("write error"){}
  };

  class read_error : public exception{
  public:
    read_error() : exception("read error"){}
  };

  class file : public opp::process{
    std::string filename;
    int fd;
  public:
    using buffer_t = std::vector<int8_t>;

    file(std::string filename, int fd);
    file(std::string filename);
    ~file();
    void replace_fd(int fd);

    virtual void loop();
    virtual void stop();

    // io funcs
    std::string readline();
    void write(const std::string &data);
    void write(buffer_t &data);
    void read(buffer_t &data);
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
