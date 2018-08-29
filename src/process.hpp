#pragma once

#include <vector>
#include <thread>
#include <utility>
#include <set>
#include <map>
#include <chrono>
#include <any>
#include <mutex>
#include <atomic>
#include <condition_variable>
#include <functional>
#include <iostream>
#include <memory>
#include "opp.hpp"

namespace opp {
  class VM;
  class process;

  struct exit_msg{ std::shared_ptr<opp::process> process; int code; };
  struct timeout_msg{ std::shared_ptr<opp::process> process; };
  struct down_msg{ std::shared_ptr<opp::process> process; };


  class process : public std::enable_shared_from_this<process>{
    std::string _name;
    std::atomic<bool> _running;
    std::atomic<bool> _inloop;
    int _pid;

    std::mutex mtx;
    std::thread thread;

    std::vector<std::any> messages;
    std::condition_variable message_signal;

    // These will receive "{DOWN, process}" when process stop running
    std::set<std::shared_ptr<process>> monitored_by;
    friend class opp::VM;
  public:
    static std::chrono::seconds FOREVER;

    process() : process("noname") {};
    process(std::string &&name);
    process(const process &) = delete;
    virtual ~process();

    const std::string &name(){ return _name; };
    int pid(){ return _pid; }

    /// Must be called to really start runnig the process. This is required for shared_ptrto work properly.
    /// It is virtual, to be able to add code that uses shared_from_this, but should call process::run() at end
    virtual void run();

    // First function to be called. Can not be on constructor as it runs on the callers context.
    virtual void loop();

    // Sends a message to this process
    void send(std::any &&msg);
    void stop();
    bool running(){ return _running; }
    void monitor();
    void demonitor();

    // FIXME. This is the lazy wait. On all send will check the full queue.
    // The nice way would be on each receive first check the message queue, and
    // then wait for my message.
    // We do it frist like this as its easier to ust check the full queue
    // in a loop.

    // returns or blocks. Filter is called on each function to know if
    // thats what you were waiting for. The process may loose the thread.
    // symbol receive(const std::map<symbol, std::function<void(const std::any &)>> &case_, const std::chrono::seconds &timeout=std::chrono::seconds(5));

    template<typename A>
    A receive(const std::chrono::seconds &timeout=std::chrono::seconds(5)){
      if (self().get() != this)
        throw_bad_receiver();

      auto until = std::chrono::system_clock::now() + timeout;

      while(true){
        // printf("%s: receive 1\n", _name.c_str());
        std::unique_lock<std::mutex> lck(mtx);
        auto endI = messages.end();
        for(auto msg=messages.begin();msg!=endI;++msg){
          if ((*msg).type() == typeid(A)) {
            A ret = std::any_cast<A>(std::move(*msg));
            messages.erase(msg);
            return ret;
          }
          maybe_exit_or_timeout(msg);
        }

        // printf("%s: Wait for message1\n", name().c_str());
        auto to_ = message_signal.wait_until(lck, until);
        if (to_ == std::cv_status::timeout)
          throw_timeout();
        if (!running())
          throw_exit(0);
      }

    }

    template<typename A, typename B>
    void receive(
                std::function<void(const A &)> fa,
                std::function<void(const B &)> fb,
                const std::chrono::seconds &timeout=std::chrono::seconds(5)
              ){
      if (self().get() != this)
        throw_bad_receiver();

      auto until = std::chrono::system_clock::now() + timeout;

      while(true){
        std::unique_lock<std::mutex> lck(mtx);
        auto endI = messages.end();
        for(auto msg=messages.begin();msg!=endI;++msg){
          bool m = match<A, B>(*msg, fa,fb);

          if (m) {
            messages.erase(msg);
            return;
          }
          maybe_exit_or_timeout(msg);
        }

        // printf("%s: Wait for message1\n", name().c_str());
        auto to_ = message_signal.wait_until(lck, until);
        if (to_ == std::cv_status::timeout)
          throw_timeout();
        if (!running())
          throw_exit(0);
      }
    }

    template<typename A>
    bool match(const std::any &msg, std::function<void(const A &)> fa){
      if (msg.type() == typeid(A)){
        fa(std::any_cast<A>(msg));
        return true;
      }
      return false;
    }
    template<typename A, typename B>
    bool match(const std::any &msg, std::function<void(const A &)> fa, std::function<void(const B &)> fb){
      bool m;
      m = match(msg, fa);
      m = m || match(msg, fb);
      return m;
    }
  private:
    void maybe_exit_or_timeout(std::vector<std::any>::iterator &);
    void base_loop();

    // I dont know about the exceptions here, as they reference the same class. Need to do some tricks.
    void throw_bad_receiver();
    void throw_timeout();
    void throw_exit(int code);
  };
}
