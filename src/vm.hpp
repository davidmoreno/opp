#pragma once

#include <mutex>
#include <set>


namespace opp{
  class process;

  class VM{
    std::mutex mutex;
    std::set<process *> processes;
  public:
    VM();
    ~VM();
    void loop();
    void loop_thread();

    process *self();
    void self(process *);

    void add_process(process *);
    void remove_process(process *);

    void print_stats();
  };

  extern VM *vm;
}
