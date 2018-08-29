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
  static thread_local std::shared_ptr<process> _self;

  VM::VM(){
    if (vm){
      throw opp::already_initialized();
    }
    vm = this;

    // Start some required classes
    opp::io::stdin = std::make_shared<opp::io::file>("stdin", 0);
    opp::io::stdin->run();

    opp::io::stdout = std::make_shared<opp::io::file>("stdout", 1);
    opp::io::stdout->run();

    opp::io::stderr = std::make_shared<opp::io::file>("stderr", 2);
    opp::io::stderr->run();

    // And self
    _self = std::make_shared<MainProcess>();
  }

  VM::~VM(){
    vm = nullptr;
  }

  void VM::loop(){

  }

  std::shared_ptr<process> VM::self(){
    return _self;
  }

  void VM::self(std::shared_ptr<process> self){
    _self = self;
  }

  void VM::add_process(std::weak_ptr<process> pr){
    std::unique_lock<std::mutex> lck(mutex);
    processes.push_back(pr);
  }
  void VM::remove_process(std::weak_ptr<process> pr){
    // std::unique_lock<std::mutex> lck(mutex);
    // processes.erase(pr);
    fprintf(stderr, "REMOVE PROCESS\n");
  }
  void VM::print_stats(){
    std::unique_lock<std::mutex> lck(mutex);
    std::ostringstream stats;
    stats<<"\n";
    stats<<"process_count: "<<processes.size()<<std::endl;
    stats<<"processes: "<<std::endl;

    for(const auto &p: processes){
      try{
        auto sp = std::shared_ptr(p);
        stats<<"  -name: "<<sp->name()<<std::endl;
      } catch (std::bad_weak_ptr &e) {
        stats<<"  -name: (removed)"<<std::endl;
      }
    }

    io::stderr->println(term::color(stats.str(), term::WHITE, term::BLUE));
  }
}
