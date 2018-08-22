#include <thread>
#include "process.hpp"
#include "vm.hpp"

namespace opp{
  process *self(){
    return vm->self();
  }

  void start(){
    new VM();
  }
  void stop(){
    if (!vm)
      throw opp::not_initialized();
    delete vm;
  }
}
