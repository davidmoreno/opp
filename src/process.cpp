#include <iostream>
#include <unistd.h>
#include <execinfo.h>
#include "process.hpp"
#include "opp.hpp"
#include "vm.hpp"

namespace opp{
  symbol EXIT("exit");
  symbol TIMEOUT("timeout");
  symbol DOWN("down");

  std::chrono::seconds process::FOREVER = std::chrono::hours(24*265*100);

  static void print_backtrace(){
    void *array[10];
    size_t size = backtrace(array, 10);
    backtrace_symbols_fd(array, size, 2);
    fprintf(stderr, "\n");
  }

  static void maybe_exit_or_timeout(const symbol &s, const std::any &msg){
    if (s == EXIT){
      if (msg.type() == typeid(process *)){
        throw opp::process_exit(std::any_cast<process*>(msg));
      }
      else{
        throw opp::process_exit(nullptr);
      }
    }
    if (s == TIMEOUT){
      if (msg.type() == typeid(process *)){
        throw opp::process_timeout(std::any_cast<process*>(msg));
      }
      else{
        throw opp::process_timeout(nullptr);
      }
    }
  }

  process::process(std::string &&name) : _name(name){
    // printf("%s: New process %p\n", _name.c_str(), this);
    _running=true;

    thread = std::move(std::thread([this]{
      try{
        // printf("%s: Start\n", this->name().c_str());
        vm->self(this);
        _inloop = true;
        this->loop();
        // printf("%s: End\n", this->name().c_str());
      } catch (std::exception &e){
        fprintf(stderr, "%s: Exit process. Exception: %s.\n", this->name().c_str(), e.what());
        print_backtrace();
      } catch (...) {
        fprintf(stderr, "%s: Exit process. Unknown exception.\n", this->name().c_str());
        print_backtrace();
      }
      _inloop = false;

      for(auto pr: monitored_by){
        pr->send(DOWN, {this});
      }
    }));

    vm->add_process(this);
  }

  process::~process(){
    // printf("%s: ~process %p\n", _name.c_str(), this);
    exit();
    while(_inloop){ // FIXME spinning to get stop
    }
    vm->remove_process(this);
    thread.join();
  }

  void process::send(const symbol &s, std::any &&msg){
    if (!running())
      throw opp::process_exit(this);
    // printf("%s: Send %s\n", name().c_str(), s.name());
    std::unique_lock<std::mutex> lck(mtx);
    messages.push_back(std::make_pair(s, msg));
    message_signal.notify_all();
  }

  void process::exit(){
    printf("%s: exit\n", _name.c_str());
    _running = false;
    send(EXIT, 0);
  }

  void process::monitor(){
    monitored_by.insert(self());
  }

  void process::demonitor(){
    monitored_by.insert(self());
  }
}
