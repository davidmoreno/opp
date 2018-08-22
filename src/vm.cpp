#include <vector>
#include <iostream>
#include <unistd.h>
#include <sstream>
#include "vm.hpp"
#include "process.hpp"
#include "io.hpp"
#include "term.hpp"

namespace opp{
  /**
   * @short This is a fake process to allow to have calls from the main process
   *
   * This is just a placeholder to allow to send and receive message from the
   * main process.
   */
  class MainProcess : public process{
  public:
    MainProcess() : process("main"){};
    virtual void loop(){
      while(running()){
        sleep(1000);
      }
    }
  };


  VM *vm = nullptr;
  static thread_local process *_self=nullptr;

  VM::VM(){
    if (vm){
      throw opp::already_initialized();
    }
    vm = this;

    // Start some required classes
    opp::io::stdin = new io::file("stdin", 0);
    opp::io::stdout = new io::file("stdout", 1);
    opp::io::stderr = new io::file("stderr", 2);

    // And self
    _self = new MainProcess();
  }

  VM::~VM(){
    vm = nullptr;
  }

  void VM::loop(){

  }

  process *VM::self(){
    return _self;
  }

  void VM::self(process *self){
    _self = self;
  }

  void VM::add_process(process *pr){
    std::unique_lock<std::mutex> lck(mutex);
    processes.insert(pr);
  }
  void VM::remove_process(process *pr){
    std::unique_lock<std::mutex> lck(mutex);
    processes.erase(pr);
  }
  void VM::print_stats(){
    std::unique_lock<std::mutex> lck(mutex);
    std::ostringstream stats;
    stats<<"\n";
    stats<<"process_count: "<<processes.size()<<std::endl;
    stats<<"processes: "<<std::endl;

    for(const auto &p: processes){
      stats<<"  -name: "<<p->name()<<std::endl;
    }

    io::stderr->println(term::color(stats.str(), term::WHITE, term::BLUE));
  }
}
