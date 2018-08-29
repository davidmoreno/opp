#include <thread>
#include "process.hpp"
#include "vm.hpp"
#include "exceptions.hpp"

namespace opp{
  std::shared_ptr<process> self(){
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
