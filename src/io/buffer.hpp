#pragma once

#include <vector>
#include <string>
#include "exceptions.hpp"

namespace opp::io{
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
    void set_size(uint32_t size){
      if (size>data_.capacity())
        throw opp::exception("Size is bigger than capacity");
      size_ = size;
    }
    int32_t size() const{
      return size_;
    }
    int32_t capacity() const{
      return data_.size();
    }
    int8_t *data(){
      return data_.data();
    }
    const int8_t *data() const{
      return data_.data();
    }
    int8_t *begin(){
      return data_.data();
    }
    int8_t *end(){
      return data_.data()+size_;
    }
    const int8_t *begin() const{
      return data_.data();
    }
    const int8_t *end() const{
      return begin()+size_;
    }
  };
}

namespace std{
  inline std::string to_string(const opp::io::buffer_t &data){
    std::string ret;
    std::copy(data.begin(), data.end(), std::back_inserter(ret));
    return ret;
  }
}
