#pragma once

#include <cxxabi.h>
#include <string>
#include <sstream>
#include <vector>

namespace opp::string{
  /// Checks teh first string starts with the second string prefix
  inline bool startswith(const std::string_view &str, const std::string_view &prefix){
    return std::mismatch(str.begin(), str.end(), prefix.begin()).second == prefix.end();
  }
  inline std::vector<std::string_view> split(std::string_view str, std::string_view separator = " "){
    std::vector<std::string_view> ret;

    int I = 0;
    int endI = str.size();
    int lastI = 0;
    bool split = false;
    for(;I!=endI;++I){
      auto c = str[I];
      for(auto sep: separator){
        if (sep == c)
          split = true;
      }

      if (split){
        if (I != lastI) // edge case, length 0
          ret.push_back(str.substr(lastI, I - lastI));
        split = false;
        lastI = I+1;
      }
    }
    if (I != lastI) // edge case, length 0
      ret.push_back(str.substr(lastI, I - lastI));

    return ret;
  }
  inline std::string_view strip(std::string_view str){
    // trim start
    auto strsize = str.size();
    decltype(strsize) start;
    for (start=0; start<strsize; start++){
      if (!isspace(str[start]))
        break;
    }
    str = str.substr(start, strsize);
    // trim end
    strsize = str.size();
    int end;
    for (end=strsize; strsize>0;){
      if (!isspace(str[end]))
        break;
      end--;
    }
    return str.substr(0, end+1);
  }
}

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
  template<typename T>
  inline std::string to_string(const std::vector<T> &data){
    std::stringstream ss;
    ss<<"[";
    for (auto &v: data){
      ss<<v<<", ";
    }
    ss<<"]";

    return ss.str();
  }

  // inline std::string to_string(const T &c){
  //   return std::string(c);
  // }

}
