#include <vector>
#include <iostream>
#include "vm.hpp"
#include "process.hpp"

namespace popc{
  static thread_local Process *_self=nullptr;

  void VM::loop(){

  }

  Process *VM::self(){
    return _self;
  }

  void VM::start_process(Process *pr){
    threads.push_back(std::move(std::thread([pr]{
      _self = pr;
      try{
        pr->process();
      } catch (const std::exception &e){
        std::cerr<<"Catch exception "<<e.what()<<std::endl;
      }
    })));
  }
}
