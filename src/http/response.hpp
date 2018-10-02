#pragma once

#include <memory>
#include <io/io.hpp>
#include <string>

namespace opp::http{
  class response{
  public:
    std::string content;
    int status_code;

    response(std::string res, int status_code_=200) : content(res), status_code(status_code_) {}

    void write_to(std::shared_ptr<opp::io::file> file);
  };
}
