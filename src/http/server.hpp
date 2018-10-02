#pragma once

#include <functional>
#include <variant>
#include "io/tcp/server.hpp"
#include "url.hpp"

namespace opp::http{
  class server : public opp::process{
    std::shared_ptr<opp::io::tcp::server> tcp_server;
    opp::http::url url;
  public:
    server(std::string address, std::string port);

    void loop();

    void set_root_url(opp::http::url url){ this->url=url; };

    void serve(std::shared_ptr<opp::io::tcp::peer> peer);
  };
}
