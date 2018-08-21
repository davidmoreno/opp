#pragma once

namespace std{
  inline std::string to_string(const char *str){
    return std::string(str);
  }
  inline std::string to_string(char str){
    return std::string(1, str);
  }

  template<typename T>
  inline std::string to_string(const T &c){
    return std::string(c);
  }
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
