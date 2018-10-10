#pragma once

#include <string>
#include "core/exceptions.hpp"

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

  class eof : public exception{
  public:
    eof() : exception("eof"){}
  };
}
