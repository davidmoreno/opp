#include <thread>
#include "process.hpp"

namespace SPC{
  Process main = Process();
  thread_local Process& _self = main;

  Process &self(){
    return _self;
  }
}
