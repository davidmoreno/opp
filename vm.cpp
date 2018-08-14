#include <vector>
#include <iostream>
#include <unistd.h>
#include "vm.hpp"
#include "process.hpp"
#include "io.hpp"

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
    virtual void process(){
      while(running()){
        sleep(1000);
      }
    }
  };


  VM *vm = nullptr;
  static thread_local Process *_self=nullptr;

  VM::VM(){
    if (vm){
      throw popc::already_initialized();
    }
    vm = this;

    // Start some required classes
    popc::IO::stdin = new IO::File("stdin", 0);
    popc::IO::stdout = new IO::File("stdout", 1);
    popc::IO::stderr = new IO::File("stderr", 2);

    // And self
    _self = new MainProcess();
  }

  VM::~VM(){
    for (auto &pt: threads){
      pt.first->exit();
    }

    vm = nullptr;
  }

  void VM::loop(){

  }

  Process *VM::self(){
    return _self;
  }

  void VM::start_process(Process *pr){
    auto thread = std::thread([pr]{
      _self = pr;
      try{
        printf("%s: Start\n", pr->name().c_str());
        pr->process();
        printf("%s: End\n", pr->name().c_str());
      } catch (std::exception &e){
        printf("%s: EXIT exception %s", pr->name().c_str(), e.what());
      } catch (...) {
        printf("%s: unkwnon exception", pr->name().c_str());
      }
      pr->exit();
    });

    threads[pr] = std::move(thread);
  }

  void VM::stop_process(Process *pr){
    pr->exit();
    threads.erase(pr);
  }
}
