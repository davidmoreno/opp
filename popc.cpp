#include <thread>
#include "process.hpp"
#include "vm.hpp"

namespace popc{
  Process *self(){
    return vm->self();
  }

  void start_process(Process *pr){
    if (!vm){
      throw popc::not_initialized();
    }
    vm->start_process(pr);
  }
  void stop_process(Process *pr){
    if (!vm){
      throw popc::not_initialized();
    }
    vm->stop_process(pr);
  }

  void start(){
    new VM();
  }
  void stop(){
    if (!vm)
      throw popc::not_initialized();
    delete vm;
  }
}
