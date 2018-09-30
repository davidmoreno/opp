#pragma once

#include "io.hpp"

namespace opp::io{
  class tcp_client : public opp::io::file{
  public:
    tcp_client(std::string address, std::string port);
  };
}
