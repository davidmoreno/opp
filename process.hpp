#pragma once

#include <vector>
#include <thread>
#include <utility>
#include <set>
#include <map>
#include <chrono>
#include <any>
#include <mutex>
#include <condition_variable>
#include <functional>
#include "symbol.hpp"

namespace popc{
  extern Symbol EXIT;
  extern Symbol TIMEOUT;

  class Process{
    std::vector<std::pair<Symbol, std::any>> messages;
    std::mutex mtx;
    std::condition_variable newmessage;
    std::string _name;
  public:
    bool running = false;

    Process();
    Process(std::string &&name);
    Process(const Process &) = delete;
    virtual ~Process();

    const std::string &name(){ return _name; };

    // First function to be called. Can not be on constructor as it runs on the callers context.
    virtual void process(){
      printf("%s: Please reimplement. EXIT now.\n", _name.c_str());
    };

    // Sends a message to this process
    void send(const Symbol &, std::any &&msg);

    // FIXME. This is the lazy wait. On all send wil lcheck the full queue.
    // The nice way would be on each receive first check the message queue, and
    // then wait for my message.
    // We do it frist like this as its easier to ust check the full queue
    // in a loop.

    // returns or blocks. Filter is called on each function to know if
    // thats what you were waiting for. The process may loose the thread.
    Symbol receive(const std::map<Symbol, std::function<void(const std::any &)>> &case_, const std::chrono::seconds &timeout=std::chrono::seconds(5));
    std::pair<Symbol, std::any> receive(const std::set<Symbol> &symbols, const std::chrono::seconds &timeout=std::chrono::seconds(5));
    std::any receive(Symbol, const std::chrono::seconds &timeout=std::chrono::seconds(5));
  };
}
