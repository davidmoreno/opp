#pragma once

#include <string>

namespace opp::http{
  class request{
  public:
    std::string method;
    std::string path;

    request(std::string &&method_, std::string &&path_) : method(method_), path(path_) {};
  };
}
