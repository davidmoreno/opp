#pragma once

#include <thread>
#include <map>

namespace popc{
  class Process;

  class VM{
    std::map<Process *, std::thread> threads;
  public:
    VM();
    ~VM();
    void loop();
    void loop_thread();

    Process *self();
    void start_process(Process *pr);
    void stop_process(Process *pr);
  };

  extern VM *vm;
}
