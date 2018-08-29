#include <iostream>
#include <unistd.h>
#include <execinfo.h>
#include "process.hpp"
#include "opp.hpp"
#include "vm.hpp"
#include "exceptions.hpp"

namespace opp{
  std::chrono::seconds process::FOREVER = std::chrono::hours(24*265*100);

  static void print_backtrace(){
    void *array[10];
    size_t size = backtrace(array, 10);
    backtrace_symbols_fd(array, size, 2);
    fprintf(stderr, "\n");
  }

  void process::loop(){
    throw opp::not_implemented();
  };

  void process::maybe_exit_or_timeout(const std::any &msg){
    if (msg.type() == typeid(exit_msg)){
      throw opp::process_exit(std::any_cast<exit_msg>(msg).process);
    }
    if (msg.type() == typeid(timeout_msg)){
      throw opp::process_timeout(std::any_cast<timeout_msg>(msg).process);
    }
  }

  process::process(std::string &&name) : _name(name){
    // printf("%s: New process %p\n", _name.c_str(), this);
    _running=false;
  }

  void process::run(){
    _running=true;
    thread = std::move(std::thread([this](){ this-> base_loop(); }));
  }

  void process::base_loop(){
    try{
      // printf("%s: Start\n", this->name().c_str());

      // here there can be a race between create the object, create the thread the shared_ptr... and it might not be ready yet.
      vm->add_process(shared_from_this());
      vm->self(shared_from_this());


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
      pr->send(down_msg{shared_from_this()});
    }
  }

  process::~process(){
    // printf("%s: ~process %p\n", _name.c_str(), this);
    exit();
    while(_inloop){ // FIXME spinning to get stop
    }
    vm->remove_process(shared_from_this());
    thread.join();
  }

  void process::send(std::any &&msg){
    if (!running())
      throw opp::process_exit(shared_from_this());
    // printf("%s: Send %s\n", name().c_str(), s.name());
    std::unique_lock<std::mutex> lck(mtx);
    messages.push_back(msg);
    message_signal.notify_all();
  }

  void process::exit(){
    printf("%s: exit\n", _name.c_str());
    _running = false;
    send(exit_msg{shared_from_this()});
  }

  void process::monitor(){
    monitored_by.insert(self());
  }

  void process::demonitor(){
    monitored_by.insert(self());
  }

  void process::throw_bad_receiver(){
    throw opp::bad_receiver();
  }
  void process::throw_timeout(){
    throw opp::process_timeout(shared_from_this());
  }
  void process::throw_exit(){
    throw opp::process_exit(shared_from_this());
  }

}
