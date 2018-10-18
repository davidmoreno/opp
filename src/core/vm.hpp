#pragma once

#include <mutex>
#include <vector>
#include <memory>

#include <boost/fiber/fss.hpp>

#include "process.hpp"
#include "scheduler.hpp"

namespace opp{
  class VM : public opp::process{
    std::mutex mutex;
    std::vector<process_t> processes;
    process_t main;
    boost::fibers::fiber_specific_ptr<process_t> _self;
    std::unique_ptr<opp::scheduler> scheduler;

    void real_stop();
  public:
    VM();
    ~VM();
    void start();
    void stop();

    void loop();
    void loop_thread();

    process_t self();
    void self(process_t pr);

    void start(process_t pr);
    void stop(process_t pr, int code);

    void print_stats();
    void clean_processes();
  };

  extern std::shared_ptr<VM> vm;
}
