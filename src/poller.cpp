#include <sys/epoll.h>

#include "poller.hpp"

std::shared_ptr<opp::poller_t> opp::poller;

opp::poller_t::poller_t(){
 pollerfd = epoll_create1(EPOLL_CLOEXEC);
 // fprintf(::stderr, "Start poller\n");

 if (pollerfd == -1){
   throw opp::exception("cant open epoll");
 }

 epoll_thread = std::thread(&poller_t::epoll_loop, this);
}

opp::poller_t::~poller_t(){
  running = false;
  close(pollerfd);
  epoll_thread.join();
}

void opp::poller_t::epoll_loop(){
  int max_events = 128;
  struct epoll_event events[max_events];

  // fprintf(::stderr, "Start polling\n");
  while(running){
    // sleep(1);
    // fprintf(::stderr, "Polling...\n");
    auto count = epoll_wait(pollerfd, events, max_events, 100000);
    // fprintf(::stderr, "Got %d\n", count);
    std::vector<boost::fibers::promise<int>> promises_to_resolve;

    {
      auto _ = std::lock_guard(pending_mutex);
      for (auto i=0;i<count;i++){
        auto fd = events[i].data.fd;
        // fprintf(::stderr, "Data ready at %d\n", fd);
        promises_to_resolve.emplace_back( std::move(pending[fd])  );
        pending.erase(fd);
      }
    }

    for(auto &promise: promises_to_resolve){
      // fprintf(::stderr, "Resolve promise\n");
      promise.set_value(1);
    }
  }
}


void opp::poller_t::wait_read(int fd){
  boost::fibers::promise<int> promise;
  auto future = promise.get_future();
  wait_fd(fd, std::move(promise), EPOLLIN|EPOLLONESHOT);
  future.wait();
}
void opp::poller_t::wait_write(int fd){
  boost::fibers::promise<int> promise;
  auto future = promise.get_future();
  wait_fd(fd, std::move(promise), EPOLLOUT|EPOLLONESHOT);
  future.wait();
}

void opp::poller_t::wait_fd(int fd, boost::fibers::promise<int> promise, int flags){
  {
    auto _ = std::lock_guard(pending_mutex);
    pending[fd] = std::move(promise);
  }
  struct epoll_event event;
  event.events = flags;
  event.data.fd = fd;
  auto error = epoll_ctl(pollerfd, EPOLL_CTL_MOD, fd, &event);
  // Should be -EINVAL ... but its not.
  if (error == -EPERM){

    // Was not added before add now. Causes 2 kernel switches first time, 1 all
    // the rest avoid problems with reuse of fds, and other modes require two
    // context switches always (add/remove) (add/mod)...

    error = epoll_ctl(pollerfd, EPOLL_CTL_ADD, fd, &event);
  }

  if(error){
    {
      auto _ = std::lock_guard(pending_mutex);
      pending.erase(fd);
    }

    throw opp::exception("failed to add fd to poller");
  }
}
