#pragma once

#include <mutex>
#include <vector>
#include <memory>
#include <atomic>
#include <any>

#include <boost/fiber/condition_variable.hpp>
#include <boost/fiber/fss.hpp>


namespace opp{
  class process;
  class vm_process;

  struct stop_process_msg{
    std::shared_ptr<process> pr;
  };

  class VM{
    std::mutex mutex;
    std::vector<std::shared_ptr<process>> processes;
    std::atomic<bool> running;
    std::shared_ptr<process> main;
    std::shared_ptr<process> vm;
    boost::fibers::fiber_specific_ptr<std::shared_ptr<process>> _self;

    // This is required to make the fiber scheduler wait on each thread until
    // I say so. https://www.boost.org/doc/libs/1_67_0/libs/fiber/examples/work_sharing.cpp
    boost::fibers::condition_variable_any running_cond{};
    std::mutex running_mutex{};
    std::vector<std::thread> workers;
    size_t nworkers;

    friend class vm_process;
    void real_stop();
  public:
    VM();
    ~VM();
    void start();
    void stop();

    void send(std::any &&msg);
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
