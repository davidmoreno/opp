#pragma once

#include "../io.hpp"

namespace opp::io::tcp{
  class peer : public file{
  public:
    peer(int fd) : file("peer", fd) {}
  };
}
