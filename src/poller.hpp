#pragma once

#include <boost/fiber/future.hpp>
#include <memory>
#include <thread>
#include <mutex>
#include <unordered_map>
#include <atomic>
#include "core/exceptions.hpp"

namespace opp {
  class poller_t : public std::enable_shared_from_this<poller_t> {
    int pollerfd;
    std::thread epoll_thread;
    std::atomic<bool> running = true;

    std::mutex pending_mutex;
    std::unordered_map<int, boost::fibers::promise<int>> pending;
  public:
    poller_t();
    ~poller_t();
    void epoll_loop();

    // To be called by a process. Waits until fd has data ready
    void wait_read(int fd);
    void wait_write(int fd);
  private:
    void wait_fd(int fd, boost::fibers::promise<int> promise, int flags);
  };

  extern std::shared_ptr<poller_t> poller;
}
