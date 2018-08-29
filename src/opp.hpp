#pragma once

#include <memory>

namespace opp{
  class process;

  void start();
  void stop();
  std::shared_ptr<process> self();
}
