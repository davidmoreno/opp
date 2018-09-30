#pragma once

#include "io.hpp"

namespace opp::io{
  class tcp_peer : public file{
  public:
    tcp_peer(int fd) : file("peer", fd) {}
  };
}
