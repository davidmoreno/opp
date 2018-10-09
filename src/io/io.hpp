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

  class buffer_t{
    std::vector<int8_t> data_;
    int32_t size_;
  public:
    buffer_t(uint32_t capacity) : data_(capacity), size_(0){}
    buffer_t(const std::string &str) : data_(str.size()), size_(str.size()){
      int i=0;
      for(auto c: str){
        data_[i++] = c;
      }
    }
    void set_size(int32_t size){
      if (size>data_.capacity())
        throw opp::exception("Size is bigger than capacity");
      size_ = size;
    }
    int32_t size(){
      return size_;
    }
    int32_t capacity(){
      return data_.size();
    }
    int8_t *data(){
      return data_.data();
    }
    int8_t *begin(){
      return data();
    }
    int8_t *end(){
      return data()+size_;
    }
    const int8_t *begin() const{
      return data_.data();
    }
    const int8_t *end() const{
      return begin()+size_;
    }
  };

  class file : public opp::process{
    std::string filename;
    int fd;
  public:
    file(std::string filename, int fd);
    file(std::string filename);
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

namespace std{
  inline std::string to_string(const opp::io::buffer_t &data){
    std::string ret;
    std::copy(data.begin(), data.end(), std::back_inserter(ret));
    return ret;
  }
}
