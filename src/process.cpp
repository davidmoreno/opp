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

  void process::maybe_exit_or_timeout(std::vector<std::any>::iterator &msgI){
    std::any &msg = *msgI;
    // fprintf(stderr, "Maybe exit or timeout %s %s %s", msg.type().name(), typeid(exit_msg).name(), typeid(timeout_msg).name());
    if (msg.type() == typeid(exit_msg)){
      auto m = std::any_cast<exit_msg>(msg);
      messages.erase(msgI);
      throw opp::process_exit(m.process, m.code);
    }
    if (msg.type() == typeid(timeout_msg)){
      auto m = std::any_cast<timeout_msg>(msg);
      messages.erase(msgI);
      throw opp::process_timeout(m.process);
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
    } catch (opp::process_exit &e){
      if (e.code != 0){
        fprintf(stderr, "\n%s: Exit process. OPP Exception: %s.\n", this->name().c_str(), e.what());
        e.print_backtrace(this->name());
      }
    } catch (opp::exception &e){
      fprintf(stderr, "\n%s: Exit process. OPP Exception: %s.\n", this->name().c_str(), e.what());
      e.print_backtrace(this->name());
    } catch (std::exception &e){
      fprintf(stderr, "\n%s: Exit process. C++ Exception: %s.\n", this->name().c_str(), e.what());
      print_backtrace(this->name());
    } catch (...) {
      fprintf(stderr, "\n%s: Exit process. Unknown exception.\n", this->name().c_str());
      print_backtrace(this->name());
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

  void process::throw_bad_cast(std::string typea, std::string typeb){
    throw opp::bad_cast(std::move(typea), std::move(typeb));
  }
  void process::throw_bad_receiver(){
    throw opp::bad_receiver(shared_from_this());
  }
  void process::throw_timeout(){
    throw opp::process_timeout(shared_from_this());
  }
  void process::throw_exit(int code){
    throw opp::process_exit(shared_from_this(), code);
  }

}
