#include <thread>
#include "process.hpp"
#include "vm.hpp"

namespace popc{
  static VM vm;

  Process *self(){
    return vm.self();
  }

  void start_process(Process *pr){
    vm.start_process(pr);
  }
}
