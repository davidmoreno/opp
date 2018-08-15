#include <thread>
#include "process.hpp"
#include "vm.hpp"

namespace popc{
  Process *self(){
    return vm->self();
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
