#pragma once

#include <boost/fiber/fiber.hpp>
#include <boost/fiber/buffered_channel.hpp>

#include <list>
#include <utility>
#include <set>
#include <map>
#include <chrono>
#include <any>
#include <atomic>
#include <functional>
#include <iostream>
#include <memory>
#include "opp.hpp"
#include "string.hpp"

namespace fibers = boost::fibers;

namespace opp {
  class VM;
  class process;

  struct exit_msg{ std::shared_ptr<opp::process> process; int code; };
  struct timeout_msg{ std::shared_ptr<opp::process> process; };
  struct down_msg{ std::shared_ptr<opp::process> process; };


  /// Checks if a given type is any of the given as template params, and returns position or <0
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

  /// Returns a string with the type names at template params
  template<typename... Args>
  inline std::string get_type_names();
  template<typename A, typename... Args>
  std::string get_type_names_helper(){
    return std::to_string(typeid(A)) + ", " + get_type_names<Args...>();
  }
  template<typename... Args>
  inline std::string get_type_names(){
    return get_type_names_helper<Args...>();
  }
  template<>
  inline std::string get_type_names(){
    return std::string("");
  }

  /// Can destructore the type of the argument of a lambda
  template <typename T>
  struct lambda_arg : public lambda_arg<decltype(&T::operator())>{};
  template <typename ClassType, typename ArgType>
  struct lambda_arg<void (ClassType::*) (ArgType args) const> {
      using type = ArgType;
  };

  class process : public std::enable_shared_from_this<process>{
    std::string _name;
    std::atomic<bool> _running;
    std::atomic<bool> _inloop;
    int _pid;

    fibers::fiber fiber;

    // Only the fiber can move the messages to the messages list
    fibers::buffered_channel<std::any> inqueue;

    std::list<std::any> messages;
    fibers::condition_variable message_signal;

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

    /// Must be called to really start runnig the process. This is required for shared_ptr to work properly.
    /// It is virtual, to be able to add code that uses shared_from_this, but should call process::run() at end
    virtual void run();

    // First function to be called. Can not be on constructor as it runs on the callers context.
    virtual void loop();

    // Sends a message to this process
    void send(std::any &&msg);
    virtual void stop();
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
    void receive(A fa, B fb,
                const std::chrono::seconds &timeout=std::chrono::seconds(5)
              ){
      if (self().get() != this)
        throw_bad_receiver();

      auto until = std::chrono::system_clock::now() + timeout;
      using TA = typename lambda_arg<A>::type;
      using TB = typename lambda_arg<B>::type;

      auto [pos, el] = get_any<TA, TB>(until);
      if (pos == 0){
        cast_n_call<TA>(std::move(el), std::move(fa));
      }
      if (pos == 1){
        cast_n_call<TB>(std::move(el), std::move(fb));
      }
    }

    template<typename A, typename B, typename C>
    void receive(A fa, B fb, C fc,
                const std::chrono::seconds &timeout=std::chrono::seconds(5)
              ){
      if (self().get() != this)
        throw_bad_receiver();

      auto until = std::chrono::system_clock::now() + timeout;
      using TA = typename lambda_arg<A>::type;
      using TB = typename lambda_arg<B>::type;
      using TC = typename lambda_arg<C>::type;

      auto [pos, el] = get_any<TA, TB, TC>(until);
      if (pos == 0){
        cast_n_call<TA>(std::move(el), std::move(fa));
      }
      if (pos == 1){
        cast_n_call<TB>(std::move(el), std::move(fb));
      }
      if (pos == 2){
        cast_n_call<TC>(std::move(el), std::move(fc));
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
      std::any msg;
      auto endI = messages.end();
#ifdef __MESSAGES_DEBUG__
      fprintf(stderr, "%s\t Wait for: %s\n", to_string().c_str(), get_type_names<Args...>().c_str());
#endif
      while(running()){
        // 1st check on messages currently on queue
        for(auto msg=messages.begin();msg!=endI;++msg){
          int pos = type_in<Args...>((*msg).type());
          if (pos>=0) {
            auto ret = std::move(*msg);
            messages.erase(msg);

            return std::make_pair(pos, std::move(ret));
          }
          maybe_exit_or_timeout(msg);
        }

        // Now get a new message or timeout
        auto res = inqueue.pop_wait_until(msg, maxt);
        if (res == fibers::channel_op_status::timeout)
          msg = timeout_msg{shared_from_this()};
        if (res != fibers::channel_op_status::success)
          throw_exit(1);
#ifdef __MESSAGES_DEBUG__
        fprintf(stderr, "%s\t Got msg %s. Waiting for %s\n",
          to_string().c_str(),
          std::to_string(msg.type()).c_str(),
          get_type_names<Args...>().c_str()
        );
#endif

        // finally set check until current first (soon second) and push at the front the new value
        // This is nice as we use lists.
        messages.push_front(std::move(msg));
        endI = messages.begin();
        ++endI;
      }
      throw_exit(0);
    }

    std::string to_string(){
      return ::opp::concat("[#", pid(), " ", name(), "]");
    }
  private:
    void maybe_exit_or_timeout(std::list<std::any>::iterator &);
    void base_loop();

    // I dont know about the exceptions here, as they reference the same class. Need to do some tricks.
    void throw_bad_cast(std::string typea, std::string typeb);
    void throw_bad_receiver();
    void throw_timeout();
    void throw_exit(int code);
  };
}

namespace std{
  inline std::string to_string(const std::shared_ptr<opp::process> &pr){
    return pr->to_string();
  }
}
