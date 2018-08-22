#pragma once

#include "process.hpp"

namespace opp::Task{
  process *start(std::function<void(void)> &&);

  class Task : public process{
    std::function<void(void)> fn;
  public:
    Task(std::function<void(void)> &&fn);

    void loop();
  };

}
