#pragma once

#include <memory>
#include "vm.hpp"

namespace opp{
  class process;
  using process_t = std::shared_ptr<process>;

  void start();
  void stop();

  template<typename A, typename... Args>
  std::shared_ptr<A> start(Args&&... args){
    auto pr = std::make_shared<A>(std::forward<Args>(args)...);
    vm->start(pr);
    return pr;
  }

  void print_backtrace(std::string name = "??");
  void print_backtrace(std::string name, void *trace[], size_t tracesize);
}
