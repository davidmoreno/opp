#include <thread>
#include "process.hpp"

namespace popc{
  thread_local Process *_self = nullptr;

  Process *self(){
    return _self;
  }
}
