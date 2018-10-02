#pragma once

#include "../io.hpp"

namespace opp::io::tcp{
  class peer;

  class server : public opp::io::file{
  public:
    server(int port) : server(fmt::format("{}", port)){};
    server(std::string port) : server("0.0.0.0", port){};
    server(std::string address, std::string port);

    void loop();

    std::shared_ptr<peer> wait_peer();
  };
}