#pragma once

#include <memory>

namespace opp{
  class process;

  void start();
  void stop();
  std::shared_ptr<process> self();

  template<typename A, typename... Args>
  std::shared_ptr<A> start(Args&&... args){
    auto pr = std::make_shared<A>(std::forward<Args>(args)...);
    pr->run();
    return pr;
  }

  void print_backtrace();
}
