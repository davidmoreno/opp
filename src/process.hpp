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
#include "symbol.hpp"
#include "opp.hpp"

namespace opp {
  class vm;

  struct exit_msg{ opp::process *process; };
  struct timeout_msg{ opp::process *process; };
  struct down_msg{ opp::process *process; };


  class process{
    std::string _name;
    std::atomic<bool> _running;
    std::atomic<bool> _inloop;

    std::mutex mtx;
    std::thread thread;

    std::vector<std::pair<symbol, std::any>> messages;
    std::condition_variable message_signal;

    // These will receive "{DOWN, process}" when process stop running
    std::set<process *> monitored_by;
    friend class vm;
  public:
    static std::chrono::seconds FOREVER;

    process() : process("noname") {};
    process(std::string &&name);
    process(const process &) = delete;
    virtual ~process();

    const std::string &name(){ return _name; };

    // First function to be called. Can not be on constructor as it runs on the callers context.
    virtual void loop(){
      throw opp::not_implemented();
    };

    // Sends a message to this process
    void send(const symbol &, std::any &&msg);
    void exit();
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
      if (self() != this)
        throw bad_receiver();

      auto until = std::chrono::system_clock::now() + timeout;

      while(true){
        // printf("%s: receive 1\n", _name.c_str());
        std::unique_lock<std::mutex> lck(mtx);
        auto endI = messages.end();
        for(auto msg=messages.begin();msg!=endI;++msg){
          if (msg->second.type() == typeid(A)) {
            A ret = std::any_cast<A>(std::move(msg->second));
            messages.erase(msg);
            return ret;
          }
          maybe_exit_or_timeout(msg);
        }

        // printf("%s: Wait for message1\n", name().c_str());
        auto to_ = message_signal.wait_until(lck, until);
        if (to_ == std::cv_status::timeout)
          throw process_timeout(this);
        if (!running())
          throw process_exit(this);

      }

    }

    template<typename A, typename B>
    symbol receive(
                std::function<void(const A &)> fa,
                std::function<void(const B &)> fb,
                const std::chrono::seconds &timeout=std::chrono::seconds(5)
              ){
      if (self() != this)
        throw bad_receiver();

      auto until = std::chrono::system_clock::now() + timeout;

      while(true){
        // printf("%s: receive 1\n", _name.c_str());
        std::unique_lock<std::mutex> lck(mtx);
        auto endI = messages.end();
        for(auto msg=messages.begin();msg!=endI;++msg){
          if (match<A,B>(msg->second, fa,fb)) {
            messages.erase(msg);
            return msg->first;
          }
          maybe_exit_or_timeout(msg);
        }

        // printf("%s: Wait for message1\n", name().c_str());
        auto to_ = message_signal.wait_until(lck, until);
        if (to_ == std::cv_status::timeout)
          throw process_timeout(this);
        if (!running())
          throw process_exit(this);

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
    void maybe_exit_or_timeout(const std::any &);
  };
}
