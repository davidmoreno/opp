#include <vector>
#include <iostream>
#include <unistd.h>
#include <sstream>
#include <pthread.h>


#include "vm.hpp"
#include "process.hpp"
#include "scheduler.hpp"
#include "term.hpp"
#include "../io/file.hpp" // FIXME separation of concerns violation
#include "../logger.hpp"

#define OPP_WORKER_THREADS 4

namespace opp{
  class main_process : public process{
  public:
    main_process() : process("main"){};
    virtual void loop(){
      throw opp::exception("Main loop invoked, and should never been. `main` is a special process that do not uses a loop.");
    }
  };

  // VM implementation

  std::shared_ptr<opp::VM> vm = nullptr;

  VM::VM() : process("vm"){
  }


  VM::~VM(){
    // fprintf(stderr, "Processes pending: %ld\n", processes.size());
  }

  void VM::start(){
    this->_running = true;
    /// Real start processing data at fibers
    scheduler = std::make_unique<::opp::scheduler>(OPP_WORKER_THREADS);
    main = std::make_shared<main_process>();
    self(main);
    // pthread_setname_np(pthread_self(), "real-main");

    // Start some required classes
    opp::io::stdin = opp::start<opp::io::file>("stdin", 0);
    opp::io::stdout = opp::start<opp::io::file>("stdout", 1);
    opp::io::stderr = opp::start<opp::io::file>("stderr", 2);
    opp::logger::__logger = opp::start<opp::logger::logger>();

    // And put me to run too
    start(shared_from_this());
  }

  void VM::stop(){
    vm->send(exit_msg{vm, 0});
    while(running()){
      // fprintf(stderr, "%s Waiting for stop %d\n", self()->to_string().c_str(), running());
      boost::this_fiber::sleep_for(std::chrono::seconds(1));
    }
    scheduler = nullptr; // Stops the scheduler

    // fprintf(stderr, "Done stop VM. All threads joined. %ld Proceeses still running.\n", processes.size());
  }

  void VM::real_stop(){
    if (self() != vm){
      throw bad_receiver{vm};
    }
    OPP_DEBUG("Stop all processes");

    // std::cerr<<"Finishing the vm. Stopping all processes."<<std::endl;

    // This is a layered shutdown.
    // It copies which process exist now (mutexed)
    // Stops them all.
    // and start again. If no processes left. Stop.
    while(true){
      sleep(1);
      std::vector<std::shared_ptr<process>> tostop;
      {
        std::unique_lock<std::mutex> lck(mutex);
        tostop = processes;
      }
      if (tostop.size() == 2) // Only vm and main left
        break;

      // fprintf(stderr, "Stop %ld processes\n", tostop.size());
      auto endI = std::rend(tostop);
      for(auto I = std::rbegin(tostop);I!=endI;++I){
        auto pr = *I;
        if (pr == vm) // not on myself (vm)
          continue;
        if (pr == main) // not on main thread (main)
          continue;
        // fprintf(stderr, "Stopping %s %d\n", pr->to_string().c_str(), pr->running());
        if (pr->running()){
          // fprintf(stderr, "Stopping %s\n", pr->to_string().c_str());
          pr->stop();
          boost::this_fiber::yield();
        }
      }
    }
    // fprintf(stderr, "All process stopped\n");
    // Manual stop main, no join
    vm->_running=false;
    main->_running=false;

    vm->send(exit_msg{vm, 0});
    main->send(exit_msg{main, 0});

    // main->message_signal.notify_one();
    // std::cerr<<"VM left "<<vm.use_count()<<std::endl;
    // fprintf(stderr, "All done\n");
  }

  void VM::loop(){
    // fprintf(stderr, "Run VM loop.\n");
    auto self = shared_from_this();
    while(true){
      receive({
        [self](exit_msg exit){
          if (exit.process == vm){
            // fprintf(::stderr, "Stop VM!\n");
            vm->real_stop();
            // fprintf(::stderr, "Throw END\n");
            throw opp::process_exit(self, 0);
          } else {
            vm->stop(exit.process, exit.code);
          }
          vm->print_stats();
        },
        [](timeout_msg){
          vm->clean_processes();
        }
      }, std::chrono::seconds(60));
    }
  }

  std::shared_ptr<process> VM::self(){
    std::shared_ptr<process> ret = *_self;
    if (ret){
      return *_self;
    }
    throw opp::not_initialized();
  }

  void VM::self(std::shared_ptr<process> self){
    _self.reset(new std::shared_ptr(self));
  }

  void VM::start(std::shared_ptr<process> pr){
    // fprintf(stderr, "Starting %s\n", pr->to_string().c_str());
    std::unique_lock<std::mutex> lck(mutex);
    processes.push_back(pr);
    pr->run();
  }

  void VM::stop(std::shared_ptr<process> pr, int code){
    // fprintf(stderr, "%s stop process %s %d\n", to_string().c_str(), pr->to_string().c_str(), code);
    if (self() != vm){
      self()->send(exit_msg{pr, code});
      return;
    }

    if (pr->running()){
      pr->send(exit_msg{pr, 0});
    }

    // Remove from running queues
    {
      std::unique_lock<std::mutex> lck(mutex);
      // std::unique_lock<std::mutex> lck(mutex);
      // processes.erase(pr);
      auto pri = find(processes.begin(), processes.end(), pr);
      if (pri == std::end(processes)){
        // fprintf(stderr, "Process not in vm: %d\n", pr->pid());
        return;
      }
      processes.erase(pri);
    }

    // fprintf(stderr, "%s wait to finish\n", pr->to_string().c_str());
    // Busy wait it to finish.
    if (pr->fiber.joinable())
      pr->fiber.join();

    // fprintf(stderr, "%s removed\n", pr->to_string().c_str());
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
        stats<<"  - name: "<<sp->name()<<std::endl;
        stats<<"    pid: "<<sp->pid()<<std::endl;
      } catch (std::bad_weak_ptr &e) {
        stats<<"  - name: (removed)"<<std::endl;
      }
    }

    fprintf(stderr, "%s\n", term::color(stats.str(), term::WHITE, term::BLUE).c_str());
  }

  /// Cleans the process list form zombi processes that are completely done.
  /// This actually just rleases one shared_ptr counter, so other
  /// users are ok continuing to use the dead zombi
  void VM::clean_processes(){
    if (!running())
      return;
    // std::cerr<<"Dirty "<<processes.size()<<std::endl;
    processes.erase(std::remove_if(processes.begin(), processes.end(), [](std::shared_ptr<process> &p){
      if (!p->running()){
        p->fiber.join();
        return true;
      }
      return false;
    }), processes.end());
    // std::cerr<<"Clean "<<processes.size()<<std::endl;
  }
}
