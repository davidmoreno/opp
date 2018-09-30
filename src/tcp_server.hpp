#pragma once

#include "io.hpp"

namespace opp::io{
  class tcp_peer;

  class tcp_server : public opp::io::file{
  public:
    tcp_server(int port) : tcp_server(fmt::format("{}", port)){};
    tcp_server(std::string port) : tcp_server("0.0.0.0", port){};
    tcp_server(std::string address, std::string port);

    void loop();

    std::shared_ptr<tcp_peer> wait_peer();
  };
}
