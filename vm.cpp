#include <vector>
#include <iostream>
#include <unistd.h>
#include <sstream>
#include "vm.hpp"
#include "process.hpp"
#include "io.hpp"

namespace opp{
  /**
   * @short This is a fake process to allow to have calls from the main process
   *
   * This is just a placeholder to allow to send and receive message from the
   * main process.
   */
  class MainProcess : public Process{
  public:
    MainProcess() : Process("main"){};
    virtual void loop(){
      while(running()){
        sleep(1000);
      }
    }
  };


  VM *vm = nullptr;
  static thread_local Process *_self=nullptr;

  VM::VM(){
    if (vm){
      throw opp::already_initialized();
    }
    vm = this;

    // Start some required classes
    opp::IO::stdin = new IO::File("stdin", 0);
    opp::IO::stdout = new IO::File("stdout", 1);
    opp::IO::stderr = new IO::File("stderr", 2);

    // And self
    _self = new MainProcess();
  }

  VM::~VM(){
    vm = nullptr;
  }

  void VM::loop(){

  }

  Process *VM::self(){
    return _self;
  }

  void VM::self(Process *self){
    _self = self;
  }

  void VM::add_process(Process *pr){
    std::unique_lock<std::mutex> lck(mutex);
    processes.insert(pr);
  }
  void VM::remove_process(Process *pr){
    std::unique_lock<std::mutex> lck(mutex);
    processes.erase(pr);
  }
  void VM::print_stats(){
    std::unique_lock<std::mutex> lck(mutex);
    std::ostringstream stats;
    stats<<"process_count: "<<processes.size()<<std::endl;
    stats<<"processes: "<<std::endl;

    for(const auto &p: processes){
      stats<<"  -name: "<<p->name()<<std::endl;
    }

    IO::stderr->print(stats.str());
  }
}
