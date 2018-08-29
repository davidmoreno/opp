#pragma once

#include <mutex>
#include <vector>
#include <memory>


namespace opp{
  class process;

  class VM{
    std::mutex mutex;
    std::vector<std::shared_ptr<process>> processes;
  public:
    VM();
    ~VM();
    void loop();
    void loop_thread();

    std::shared_ptr<process> self();
    void self(std::shared_ptr<process> pr);

    void start(std::shared_ptr<process> pr);
    void stop(std::shared_ptr<process> pr);

    void print_stats();
  };

  extern VM *vm;
}
