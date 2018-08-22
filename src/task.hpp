#pragma once

#include "process.hpp"

namespace opp::Task{
  Process *start(std::function<void(void)> &&);

  class Task : public Process{
    std::function<void(void)> fn;
  public:
    Task(std::function<void(void)> &&fn);

    void loop();
  };

}
