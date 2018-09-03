#pragma once

#include <cxxabi.h>

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

  // inline std::string to_string(const T &c){
  //   return std::string(c);
  // }

}


namespace opp{
  template<typename T>
  std::string concat(T v){
    return std::to_string(v);
  }
  template<typename T, typename... Args>
  std::string concat(T &first, Args... args){
    return std::to_string(first) + concat(args...);
  }
}
