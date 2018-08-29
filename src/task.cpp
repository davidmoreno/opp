#include "task.hpp"
#include "logger.hpp"

namespace opp::task{

  std::shared_ptr<task> start(std::function<void(void)> &&fn){
    auto task = opp::start<opp::task::task>(std::move(fn));
    return task;
  }

  task::task(std::function<void(void)> &&_fn) : process("task"), fn(_fn){}

  void task::loop(){
    OPP_DEBUG("Start task");
    fn();
    OPP_DEBUG("End task");
  }
}
