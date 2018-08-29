#pragma once

#include "process.hpp"

namespace opp::task{
  class task : public process{
    std::function<void(void)> fn;
  public:
    task(std::function<void(void)> &&fn);

    void loop();
  };

  std::shared_ptr<task> start(std::function<void(void)> &&);
}
