#include <vector>
#include <iostream>
#include <unistd.h>
#include "vm.hpp"
#include "process.hpp"

namespace popc{
  /**
   * @short This is a fake process to allow to have calls from the main process
   *
   * This is just a placeholder to allow to send and receive message from the
   * main process.
   */
  class MainProcess : public Process{
  public:
    MainProcess() : Process("main"){};
    void process(){
      while(running){
        sleep(1000);
      }
    }
  };

  static thread_local Process *_self=nullptr;

  VM::VM(){
    _self = new MainProcess();
  }

  void VM::loop(){

  }

  Process *VM::self(){
    return _self;
  }

  void VM::start_process(Process *pr){
    threads.push_back(std::move(std::thread([pr]{
      _self = pr;
      try{
        printf("%s: Start\n", pr->name().c_str());
        pr->process();
        printf("%s: End\n", pr->name().c_str());
      } catch (const std::exception &e){
        printf("%s: EXIT exception %s", pr->name().c_str(), e.what());
      }
    })));
  }


}
