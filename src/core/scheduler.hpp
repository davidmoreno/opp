#pragma once

#include <mutex>
#include <thread>
#include <condition_variable>
#include <boost/intrusive_ptr.hpp>

#include <boost/fiber/condition_variable.hpp>

namespace opp{
  /**
   * @short Custom scheduler heavily inspired on work_stealing from boost
   *
   */
  class scheduler{
    boost::fibers::condition_variable_any running_cond{};
    std::mutex running_mutex{};
    std::vector<std::thread> workers;
    size_t nworkers;
    std::atomic<bool> _running;
  public:
    scheduler( std::uint32_t threads );
    ~scheduler();

    scheduler( scheduler const&) = delete;
    scheduler( scheduler &&) = delete;
    scheduler & operator=( scheduler const&) = delete;
    scheduler & operator=( scheduler &&) = delete;
  };
}
