#pragma once

#include "process.hpp"

namespace popc{
  Process *self();

  void start_process(Process *pr);
}
