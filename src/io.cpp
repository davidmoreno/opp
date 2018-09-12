#include <iostream>
#include <unistd.h>
#include <memory.h>
#include <sys/epoll.h>
#include "io.hpp"
#include "core/opp.hpp"
#include "logger.hpp"
#include "core/exceptions.hpp"

namespace opp::io{
  class poller_t : public std::enable_shared_from_this<poller_t> {
    int pollerfd;
    std::thread epoll_thread;
    std::atomic<bool> running = true;

    std::mutex pending_mutex;
    std::unordered_map<int, std::function<void(void)>> pending;
  public:
    poller_t() {
      pollerfd = epoll_create1(EPOLL_CLOEXEC);
      fprintf(::stderr, "Start poller\n");

      if (pollerfd == -1){
        throw opp::exception("cant open epoll");
      }

      epoll_thread = std::thread(&poller_t::epoll_loop, this);
    }
    ~poller_t(){
      running = false;
      close(pollerfd);
      epoll_thread.join();
    }
    void epoll_loop(){
      int max_events = 128;
      struct epoll_event events[max_events];

      fprintf(::stderr, "Start polling\n");
      while(running){
        fprintf(::stderr, "Polling...\n");
        auto count = epoll_wait(pollerfd, events, max_events, 100000);
        fprintf(::stderr, "Got %d\n", count);
        std::vector<std::function<void(void)>> to_call;

        {
          auto _ = std::lock_guard(pending_mutex);
          for (auto i=0;i<count;i++){
            auto fd = events[i].data.fd;
            to_call.emplace_back( std::move(pending[fd])  );
            pending.erase(fd);
          }
        }

        for(auto cb: to_call){
          try{
            cb();
          } catch (...) {
            fprintf(::stderr, "Error processing io!\n");
          }
        }
      }
    }

    // To be called by a process
    void wait_read(int fd){

    }
    void wait_write(int fd){

    }
  private:
    void wait_read_fd(int fd, std::function<void(void)> callback){
      wait_fd(fd, std::move(callback), EPOLLOUT);
    }
    void wait_write_fd(int fd, std::function<void(void)> callback){
      wait_fd(fd, std::move(callback), EPOLLIN);
    }
    void wait_fd(int fd, std::function<void(void)> callback, int flags){
      {
        auto _ = std::lock_guard(pending_mutex);
        pending[fd] = callback;
      }
      struct epoll_event event;
      event.events = flags;
      event.data.fd = fd;
      if(epoll_ctl(pollerfd, EPOLL_CTL_ADD, 0, &event)){
        throw opp::exception("failed to add fd to poller");
      }
    }
  };

  static std::shared_ptr<poller_t> poller;


  struct print_msg{ std::string str; };
  struct readline_msg{ std::shared_ptr<opp::process> from; };
  struct readline_result_msg{ std::string string; };

  std::shared_ptr<file> stdin;
  std::shared_ptr<file> stdout;
  std::shared_ptr<file> stderr;

  file::file(std::string &&name, int fd) : process(std::string(name)), filename(name), fd(fd){
    if (poller.use_count() == 0){ // Initialize once
      poller = std::make_shared<poller_t>();
    }
  }

  file::~file(){
    close(fd);
  }

  void file::loop(){
    // printf("process file %s %d\n", filename.c_str(), fd);
    // Build once, use man times
    auto printfn = [this](print_msg msg){
      auto str = msg.str;
      auto wrote = write(this->fd, str.c_str(), str.size());
      if ((wrote < 0) || (unsigned(wrote) < str.size())){
        throw write_error();
      }
    };
    auto readlinefn = [this](readline_msg msg){
      std::string ret="";
      char c;
      do{
        ssize_t n = read(fd, &c, 1);
        if (n!=1){
          OPP_DEBUG("stdin closed");
          msg.from->send(exit_msg{shared_from_this()});
          stop();
          return;
        }
        ret+=c;
      }while(c!='\n');
      msg.from->send(readline_result_msg{ret});
    };

    // std::map<symbol, std::function<void(const std::any &)>> _case = {
    //   {PRINT, [&printfn](const std::any &args){
    //     auto msg = std::any_cast<print_msg>(args);
    //     printfn(msg);;
    //   }},
    //   {READLINE, [&readlinefn](const std::any &args){
    //     auto msg = std::any_cast<readline_msg>(args);
    //     readlinefn(msg);
    //     // printf("%s: Answer for %s\n", name().c_str(), from->name().c_str());
    //   }}
    // };

    while(running()){ // This will exit because of an exception when closed
      receive(printfn, readlinefn, FOREVER);
    }
  }

  void file::stop(){
    close(fd);
    fprintf(::stderr, "Force stop %s\n", name().c_str());
    this->process::stop();
  }

  void file::print_(std::string &&str){
    send(print_msg{str});
  }

  std::string file::readline(){
    send(readline_msg{opp::self()});
    auto res = opp::self()->receive<readline_result_msg>(process::FOREVER);
    return res.string;
  }
}
