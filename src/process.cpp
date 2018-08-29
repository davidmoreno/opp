#include <iostream>
#include <unistd.h>
#include <execinfo.h>
#include "process.hpp"
#include "opp.hpp"
#include "vm.hpp"
#include "exceptions.hpp"

namespace opp{
  std::chrono::seconds process::FOREVER = std::chrono::hours(24*265*100);
  static std::atomic<long> pidcount = 0;

  void process::loop(){
    throw opp::not_implemented();
  };

  void process::maybe_exit_or_timeout(const std::any &msg){
    // fprintf(stderr, "Maybe exit or timeout %s %s %s", msg.type().name(), typeid(exit_msg).name(), typeid(timeout_msg).name());
    if (msg.type() == typeid(exit_msg)){
      throw opp::process_exit(std::any_cast<exit_msg>(msg).process, 1);
    }
    if (msg.type() == typeid(timeout_msg)){
      throw opp::process_timeout(std::any_cast<timeout_msg>(msg).process);
    }
  }

  process::process(std::string &&name) : _name(name){
    // printf("%s: New process %p\n", _name.c_str(), this);
    _running=false;
    _pid = ++pidcount;
  }

  void process::run(){
    _running=true;
    thread = std::move(std::thread([this](){ this-> base_loop(); }));
  }

  void process::base_loop(){
    try{
      // printf("%s: Start\n", this->name().c_str());

      // here there can be a race between create the object, create the thread the shared_ptr... and it might not be ready yet.
      vm->self(shared_from_this());


      _inloop = true;
      this->loop();
      // printf("%s: End\n", this->name().c_str());
    } catch (std::exception &e){
      fprintf(stderr, "\n%s: Exit process. Exception: %s.\n", this->name().c_str(), e.what());
      print_backtrace();
    } catch (...) {
      fprintf(stderr, "\n%s: Exit process. Unknown exception.\n", this->name().c_str());
      print_backtrace();
    }
    _inloop = false;

    for(auto pr: monitored_by){
      pr->send(down_msg{shared_from_this()});
    }
  }

  process::~process(){
  }

  void process::send(std::any &&msg){
    if (!running())
      throw opp::process_exit(shared_from_this(), 1);
    // printf("%s: Send %s\n", name().c_str(), s.name());
    std::unique_lock<std::mutex> lck(mtx);
    messages.push_back(msg);
    message_signal.notify_all();
  }

  void process::stop(){
    printf("%s: stop\n", _name.c_str());
    vm->stop(shared_from_this());
  }

  void process::monitor(){
    monitored_by.insert(self());
  }

  void process::demonitor(){
    monitored_by.insert(self());
  }

  void process::throw_bad_receiver(){
    throw opp::bad_receiver(shared_from_this());
  }
  void process::throw_timeout(){
    throw opp::process_timeout(shared_from_this());
  }
  void process::throw_exit(){
    throw opp::process_exit(shared_from_this(), 1);
  }

}
