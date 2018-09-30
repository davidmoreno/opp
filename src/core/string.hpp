#pragma once

#include <cxxabi.h>
#include <string>
#include <vector>

namespace std{
  inline std::string to_string(const char *str){
    return std::string(str);
  }
  inline std::string to_string(const char str){
    return std::string(1, str);
  }
  inline std::string to_string(std::string str){
    return std::move(str);
  }
  inline std::string to_string(const std::type_info &ti){
    char tmp[256];
    size_t size = sizeof(tmp);
    int _status;
    const char *res = abi::__cxa_demangle(ti.name(), tmp, &size, &_status);
    if (res)
      return std::string(res);
    return ti.name();
  }
  inline std::string to_string(const std::vector<uint8_t> &data){
    std::string ret;
    std::copy(data.begin(), data.end(), std::back_inserter(ret));
    return ret;
  }

  // inline std::string to_string(const T &c){
  //   return std::string(c);
  // }

}
