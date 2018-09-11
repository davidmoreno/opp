#include <vector>
#include <iostream>
#include <unistd.h>
#include <sstream>
#include <pthread.h>

#include <boost/fiber/barrier.hpp>
#include <boost/fiber/algo/shared_work.hpp>
#include <boost/fiber/algo/work_stealing.hpp>

#include "vm.hpp"
#include "process.hpp"
#include "term.hpp"
#include "../io.hpp" // FIXME separation of concerns violation
#include "../logger.hpp"
#include "extra/thread_barrier.hpp"

#define OPP_WORKER_THREADS 4

namespace opp{

  /**
   * @short This is a fake process to allow to have calls from the main process
   *
   * This is just a placeholder to allow to send and receive message from the
   * main process.
   */
  class vm_process : public process{
  public:
    vm_process() : process("vm"){}
    virtual void loop(){
      while(running()){
        receive(
          [](exit_msg){
            vm->real_stop();
          },
          [](timeout_msg){
            vm->clean_processes();
          },
          [](stop_process_msg msg){
            fprintf(stderr, "Stop process message %s\n", msg.pr->to_string().c_str());
          },
          std::chrono::seconds(60)
        );
      }
    }
  };

  class main_process : public process{
  public:
    main_process() : process("main"){};
    virtual void loop(){
      boost::this_fiber::sleep_for(std::chrono::seconds(100));
      // receive<exit_msg>(process::FOREVER);
    }
  };

  std::shared_ptr<opp::VM> vm = nullptr;

  VM::VM() : nworkers(OPP_WORKER_THREADS){
  }


  VM::~VM(){
  }

  void VM::send(std::any &&msg){
    vm->send(std::move(msg));
  }

  void VM::start(){
    running = true;

    /// Real start processing data at fibers

    thread_barrier barrier(nworkers);
    boost::fibers::use_scheduling_algorithm< boost::fibers::algo::work_stealing >(nworkers, true);
    for (int i=0; i<nworkers-1; ++i){
      workers.emplace_back([this, &barrier](){
        boost::fibers::use_scheduling_algorithm< boost::fibers::algo::work_stealing>(nworkers, true);
        barrier.wait();

        std::unique_lock<std::mutex> lk(running_mutex);
        // this is what makes all the other fibers to run, or this thread to wait.
        running_cond.wait( lk, [this](){ return !this->running; } );
        printf("EOT\n");
      });
    }
    barrier.wait();

    main = opp::start<main_process>();
    self(main);
    vm = opp::start<vm_process>();
    // pthread_setname_np(pthread_self(), "real-main");

    // Start some required classes
    opp::io::stdin = opp::start<opp::io::file>("stdin", 0);
    opp::io::stdout = opp::start<opp::io::file>("stdout", 1);
    opp::io::stderr = opp::start<opp::io::file>("stderr", 2);
    opp::logger::__logger = opp::start<opp::logger::logger>();


  }

  void VM::stop(){
    vm->send(exit_msg{vm, 0});
    while(vm->running()){
      sleep(1);
    }

    running=false;
    running_cond.notify_all();
    for (std::thread & t: workers) { /*< wait for threads to terminate >*/
      t.join();
    }
    fprintf(stderr, "Done stop VM\n");
  }

  void VM::real_stop(){
    if (self() != vm){
      throw bad_receiver{vm};
    }
    OPP_DEBUG("Stop all processes");

    // std::cerr<<"Finishing the vm. Stopping all processes."<<std::endl;

    running = false;
    // This is a layered shutdown.
    // It copies which process exist now (mutexed)
    // Stops them all.
    // and start again. If no processes left. Stop.
    while(true){
      std::vector<std::shared_ptr<process>> tostop;
      {
        std::unique_lock<std::mutex> lck(mutex);
        tostop = processes;
      }
      if (tostop.size() == 2) // Only vm and main left
        break;

      fprintf(stderr, "Stop %ld processes\n", tostop.size());
      auto endI = std::rend(tostop);
      for(auto I = std::rbegin(tostop);I!=endI;++I){
        auto pr = *I;
        if (pr == vm) // not on myself (vm)
          continue;
        if (pr == main) // not on main thread (main)
          continue;
        if (pr->running()){
          std::cerr<<"Stopping "<<to_string(pr)<<std::endl;
          pr->stop();
        }
      }
    }
    // Manual stop main, no join
    vm->_running=false;
    main->_running=false;
    // main->message_signal.notify_one();
    // std::cerr<<"VM left "<<vm.use_count()<<std::endl;
  }

  void VM::loop(){

  }

  std::shared_ptr<process> VM::self(){
    return *_self;
  }

  void VM::self(std::shared_ptr<process> self){
    _self.reset(new std::shared_ptr(self));
  }

  void VM::start(std::shared_ptr<process> pr){
    std::unique_lock<std::mutex> lck(mutex);
    processes.push_back(pr);
    pr->run();
  }

  void VM::stop(std::shared_ptr<process> pr){
    if (self() != main){
      throw opp::bad_receiver(main);
    }

    if (pr->running()){
      pr->_running = false;
      pr->message_signal.notify_all();
    }

    // Remove from running queues
    {
      std::unique_lock<std::mutex> lck(mutex);
      // std::unique_lock<std::mutex> lck(mutex);
      // processes.erase(pr);
      auto pri = find(processes.begin(), processes.end(), pr);
      if (pri == std::end(processes)){
        fprintf(stderr, "Process not in vm: %d\n", pr->pid());
        return;
      }
      processes.erase(pri);
    }

    // Busy wait it to finish.
    if (pr->fiber.joinable())
      pr->fiber.join();
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

    fprintf(stderr, "%s\n", term::color(stats.str(), term::WHITE, term::BLUE).c_str());
  }

  /// Cleans the process list form zombi processes that are completely done.
  /// This actually just rleases one shared_ptr counter, so other
  /// users are ok continuing to use the dead zombi
  void VM::clean_processes(){
    if (!running)
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
