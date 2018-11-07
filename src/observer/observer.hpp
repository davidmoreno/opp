#pragma once

#include <process.hpp>
#include <http/server.hpp>
#include <http/url.hpp>
#include <io/buffer.hpp>

namespace opp{
  class observer : public opp::process {
    std::shared_ptr<http::server> http_server;
    io::buffer_t html;
  public:
    observer(std::string &&uipath, std::string &&address, std::string &&port);

    void loop();

    http::response index(const http::request &req);
    http::response api(const http::request &req);
  };
}
