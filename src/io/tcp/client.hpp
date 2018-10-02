#pragma once

#include "../io.hpp"

namespace opp::io::tcp{
  class client : public opp::io::file{
  public:
    client(std::string address, std::string port);
  };
}
