#pragma once

#include "../file.hpp"

namespace opp::io::tcp{
  class peer : public file{
  public:
    peer(int fd) : file(fmt::format("peer {}", fd), fd) {}
  };
}
