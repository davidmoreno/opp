#pragma once

#include <mutex>
#include <vector>
#include <memory>
#include <atomic>


namespace opp{
  class process;

  class VM{
    std::mutex mutex;
    std::vector<std::shared_ptr<process>> processes;
    std::atomic<bool> running;

  public:
    VM();
    ~VM();
    void start();
    void stop();

    void loop();
    void loop_thread();

    std::shared_ptr<process> self();
    void self(std::shared_ptr<process> pr);

    void start(std::shared_ptr<process> pr);
    void stop(std::shared_ptr<process> pr);

    void print_stats();
    void clean_processes();
  };

  extern std::shared_ptr<VM> vm;
}
