#pragma once

#include <exception>

namespace popc{
  class Process;
  
  Process *self();

  void start_process(Process *pr);

  class exception : public std::exception {};
}
