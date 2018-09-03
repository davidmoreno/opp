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


  template<typename... Args>
  int type_in(const std::type_info &tpe);

  template<typename A, typename... Args>
  int type_in_helper(const std::type_info &tpe){
    if (tpe == typeid(A))
      return 0;
    return type_in<Args...>(tpe) + 1;
  }
  template<typename... Args>
  int type_in(const std::type_info &tpe){
    return type_in_helper<Args...>(tpe);
  }
  template<>
  inline int type_in<>(const std::type_info &tpe){
    return -10000;
  }


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
    // We do it first like this as its easier to ust check the full queue
    // in a loop.

    // returns or blocks. Filter is called on each function to know if
    // thats what you were waiting for. The process may loose the thread.
    template<typename A>
    A receive(const std::chrono::seconds &timeout=std::chrono::seconds(5)){
      if (self().get() != this)
        throw_bad_receiver();

      auto until = std::chrono::system_clock::now() + timeout;

      int pos;
      std::any el;
      std::tie(pos, el) = get_any<A>(until);
      try{
        return std::any_cast<A>(el);
      } catch(...){
        throw_bad_cast(el.type().name(), typeid(A).name());
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

      int pos;
      std::any el;
      std::tie(pos, el) = get_any<A,B>(until);
      if (pos == 0){
        cast_n_call<A>(std::move(el), std::move(fa));
      }
      if (pos == 1){
        cast_n_call<B>(std::move(el), std::move(fb));
      }
    }

    template<typename A>
    void cast_n_call(std::any &&el, std::function<void(const A &)> f){
      A ael;
      try{
        ael=std::any_cast<A>(std::move(el));
      } catch (...){
        throw_bad_cast(el.type().name(), typeid(A).name());
      }
      f(ael);
    }

    /**
     * @short Gets from the queue any message from the template list
     */
    template<typename... Args>
    std::pair<int, std::any> get_any(const std::chrono::time_point<std::chrono::system_clock> &maxt){
      while(running()){
        std::unique_lock<std::mutex> lck(mtx);
        auto endI = messages.end();
        for(auto msg=messages.begin();msg!=endI;++msg){
          int pos = type_in<Args...>((*msg).type());
          if (pos>=0) {
            auto ret = std::move(*msg);
            messages.erase(msg);
            return std::make_pair(pos, ret);
          }
          maybe_exit_or_timeout(msg);
        }
        auto to_ = message_signal.wait_until(lck, maxt);
        if (to_ == std::cv_status::timeout)
          throw_timeout();
      }
      throw_exit(0);
    }
  private:
    void maybe_exit_or_timeout(std::vector<std::any>::iterator &);
    void base_loop();

    // I dont know about the exceptions here, as they reference the same class. Need to do some tricks.
    void throw_bad_cast(std::string typea, std::string typeb);
    void throw_bad_receiver();
    void throw_timeout();
    void throw_exit(int code);
  };
}
