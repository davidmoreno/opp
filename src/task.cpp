#include "task.hpp"
#include "logger.hpp"

namespace opp::Task{

  process *start(std::function<void(void)> &&fn){
    auto task = new Task(std::move(fn));
    return task;
  }

  Task::Task(std::function<void(void)> &&_fn) : fn(_fn){}

  void Task::loop(){
    OPP_DEBUG("Start task");
    fn();
    OPP_DEBUG("End task");
  }
}
