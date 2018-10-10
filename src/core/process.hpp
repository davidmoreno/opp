#pragma once

#include <boost/fiber/fiber.hpp>
#include <boost/fiber/buffered_channel.hpp>
#include <boost/fiber/condition_variable.hpp>

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
#include <fmt/format.h>
#include "string.hpp"
#include "match.hpp"
#include "utils.hpp"

namespace fibers = boost::fibers;

namespace opp {
  class VM;
  class process;

  std::shared_ptr<opp::process> self();

  struct exit_msg{ std::shared_ptr<opp::process> process; int code; };
  struct timeout_msg{ std::shared_ptr<opp::process> process; };
  struct down_msg{ std::shared_ptr<opp::process> process; };

  class process : public std::enable_shared_from_this<process>{
    std::string _name;
    std::atomic<bool> _running;
    std::atomic<bool> _inloop;
    std::atomic<bool> _debug = false;
    int _pid;

    fibers::fiber fiber;

    // Only the fiber can move the messages to the messages list
    fibers::buffered_channel<std::any> inqueue;

    std::list<std::any> messages;

    // These will receive "{DOWN, process}" when process stop running
    std::set<std::shared_ptr<process>> monitored_by;
    friend class opp::VM;
  public:
    static std::chrono::seconds FOREVER;

    process() : process("noname") {};
    process(std::string name);
    process(const process &) = delete;
    virtual ~process();

    const std::string &name(){ return _name; };
    int pid(){ return _pid; }
    void set_debug(bool debug){ _debug = debug; }
    bool debug(){ return _debug; }

    // First function to be called. Can not be on constructor as it runs on the callers context.
    virtual void loop();

    // Sends a message to this process
    void send(std::any &&msg);
    void stop(int code=0);
    bool running(){ return _running; }
    void monitor();
    void demonitor();

    std::any receive(
        const std::initializer_list<match_case> cases,
        const std::chrono::seconds &timeout=std::chrono::seconds(5));

    std::string to_string(){
      return fmt::format("[#{} {}]", pid(), name());
    }

    /// Set to true to do not show any exception data at exit, just exit.
    struct{
      bool silent_exception_exit = false;
    } flags;
  private:
    void maybe_exit_or_timeout(std::list<std::any>::iterator &);
    void maybe_exit_or_timeout(const std::any &);
    void base_loop();

    // I dont know about the exceptions here, as they reference the same class. Need to do some tricks.
    void throw_bad_cast(std::string typea, std::string typeb);
    void throw_bad_receiver();
    void throw_timeout();
    void throw_exit(int code);
    void run();

  };
}

namespace std{
  inline std::string to_string(const std::shared_ptr<opp::process> &pr){
    return pr->to_string();
  }
}
