#pragma once

#include <vector>
#include <thread>
#include <utility>
#include <set>
#include <map>
#include <chrono>
#include <any>
#include <functional>
#include "symbol.hpp"

namespace SPC{
  class Process{
    std::vector<std::pair<Symbol, std::any>> messages;
  public:
    bool running = false;

    Process();
    virtual ~Process();

    // First function to be called. Can not be on constructor as it runs on the callers context.
    virtual void process() = 0;

    // Sends a message to this process
    void send(const Symbol &, std::any &&msg);
    // returns or blocks. Filter is called on each function to know if
    // thats what you were waiting for. The process may loose the thread.
    std::pair<Symbol, std::any> receive(const std::map<Symbol, std::function<void(const std::any &)>> &case_, const std::chrono::seconds &timeout=std::chrono::seconds(5));
    std::pair<Symbol, std::any> receive(const std::set<Symbol> &symbols, const std::chrono::seconds &timeout=std::chrono::seconds(5));
    std::any receive(Symbol, const std::chrono::seconds &timeout=std::chrono::seconds(5));
  };
}
