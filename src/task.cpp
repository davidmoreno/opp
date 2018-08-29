#include "task.hpp"
#include "logger.hpp"

namespace opp::task{

  std::shared_ptr<task> start(std::function<void(void)> &&fn){
    auto task = std::make_shared<opp::task::task>(std::move(fn));
    task->run();
    return task;
  }

  task::task(std::function<void(void)> &&_fn) : fn(_fn){}

  void task::loop(){
    OPP_DEBUG("Start task");
    fn();
    OPP_DEBUG("End task");
  }
}
