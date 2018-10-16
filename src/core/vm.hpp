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
    std::vector<std::shared_ptr<opp::process>> processes;
    std::shared_ptr<opp::process> main;
    boost::fibers::fiber_specific_ptr<std::shared_ptr<process>> _self;
    std::unique_ptr<opp::scheduler> scheduler;

    void real_stop();
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
    void stop(std::shared_ptr<process> pr, int code);

    void print_stats();
    void clean_processes();
  };

  extern std::shared_ptr<VM> vm;
}
