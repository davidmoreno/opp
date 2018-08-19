#pragma once

#include <mutex>
#include <set>


namespace opp{
  class Process;

  class VM{
    std::mutex mutex;
    std::set<Process *> processes;
  public:
    VM();
    ~VM();
    void loop();
    void loop_thread();

    Process *self();
    void self(Process *);
  };

  extern VM *vm;
}
