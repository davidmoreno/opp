#include <vector>
#include <iostream>
#include <unistd.h>
#include <sstream>
#include "vm.hpp"
#include "process.hpp"
#include "io.hpp"
#include "term.hpp"
#include "logger.hpp"

namespace opp{

  /**
   * @short This is a fake process to allow to have calls from the main process
   *
   * This is just a placeholder to allow to send and receive message from the
   * main process.
   */
  class main_process : public process{
  public:
    main_process() : process("main"){};
    virtual void loop(){
      while(running()){
        sleep(1);
        vm->clean_processes();
      }
    }
  };


  std::shared_ptr<opp::VM> vm = nullptr;
  static thread_local std::shared_ptr<process> _self;

  VM::VM(){
  }


  VM::~VM(){
  }


  void VM::start(){
    running = true;
    _self = opp::start<main_process>();

    // Start some required classes
    opp::io::stdin = opp::start<opp::io::file>("stdin", 0);
    opp::io::stdout = opp::start<opp::io::file>("stdout", 1);
    opp::io::stderr = opp::start<opp::io::file>("stderr", 2);

    opp::logger::__logger = opp::start<opp::logger::logger>();
  }

  void VM::stop(){
    std::cerr<<"Finishing the vm. Stopping all processes."<<std::endl;

    running = false;
    std::unique_lock<std::mutex> lck(mutex);
    for(auto pr: processes){
      if (pr->running()){
        std::cerr<<"Stopping #"<<pr->pid()<<" <"<<pr->name()<<">"<<std::endl;
        pr->_running = false;
        pr->message_signal.notify_all();
        pr->thread.join();
      }
    }
    processes.clear();
    // std::cerr<<"VM left "<<vm.use_count()<<std::endl;
  }

  void VM::loop(){

  }

  std::shared_ptr<process> VM::self(){
    return _self;
  }

  void VM::self(std::shared_ptr<process> self){
    _self = self;
  }

  void VM::start(std::shared_ptr<process> pr){
    std::unique_lock<std::mutex> lck(mutex);
    processes.push_back(pr);
    pr->run();
  }
  void VM::stop(std::shared_ptr<process> pr){
    std::unique_lock<std::mutex> lck(mutex);
    // std::unique_lock<std::mutex> lck(mutex);
    // processes.erase(pr);
    auto pri = find(processes.begin(), processes.end(), pr);
    if (pri == std::end(processes)){
      fprintf(stderr, "Process not in vm: %d\n", pr->pid());
      return;
    }
    processes.erase(pri);

    pr->_running = false;
    // Busy wait it to finish.
    pr->thread.join();
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

  /// Cleans the process list form zombi processes that are completely done.
  /// This actually just rleases one shared_ptr counter, so other
  /// users are ok continuing to use the dead zombi
  void VM::clean_processes(){
    if (!running)
      return;
    // std::cerr<<"Dirty "<<processes.size()<<std::endl;
    processes.erase(std::remove_if(processes.begin(), processes.end(), [](std::shared_ptr<process> &p){
      return !p->running();
    }), processes.end());
    // std::cerr<<"Clean "<<processes.size()<<std::endl;
  }
}
