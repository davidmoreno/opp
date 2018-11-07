#pragma once

#include <memory>
#include <io/file.hpp>
#include <io/buffer.hpp>
#include <string>

namespace opp::http{
  class response{
  public:
    io::buffer_t content;
    int status_code;
    std::unordered_map<std::string, std::string> headers;

    response(io::buffer_t buf, int status_code_=200) : content(std::move(buf)), status_code(status_code_) {
      headers["Content-Type"] = "text/html";
      headers["Connection"] = "keep-alive";
    };
    response(std::string res, int status_code_=200) : content(res), status_code(status_code_) {
      headers["Content-Type"] = "text/html";
      headers["Connection"] = "keep-alive";
    }

    void write_to(std::shared_ptr<opp::io::file> file);
  };
}
