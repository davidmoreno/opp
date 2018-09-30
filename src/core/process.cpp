#include <iostream>
#include <unistd.h>
#include <execinfo.h>
#include "process.hpp"
#include "opp.hpp"
#include "vm.hpp"
#include "exceptions.hpp"

#define OPP_MAX_CHANNEL_SIZE 128

namespace opp{
  std::chrono::seconds process::FOREVER = std::chrono::hours(24*265*100);
  static std::atomic<long> pidcount = 0;

  void process::loop(){
    throw opp::not_implemented();
  };

  void process::maybe_exit_or_timeout(std::list<std::any>::iterator &msgI){
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
  void process::maybe_exit_or_timeout(const std::any &msg){
    if (msg.type() == typeid(exit_msg)){
      auto m = std::any_cast<exit_msg>(msg);
      throw opp::process_exit(m.process, m.code);
    }
    if (msg.type() == typeid(timeout_msg)){
      auto m = std::any_cast<timeout_msg>(msg);
      throw opp::process_timeout(m.process);
    }
  }

  process::process(std::string name) : _name(std::move(name)), inqueue(OPP_MAX_CHANNEL_SIZE){
    // printf("%s: New process %p\n", _name.c_str(), this);
    _running=false;
    _pid = ++pidcount;
    _running=true;
    // pthread_setname_np(pthread_self(), name.c_str());
  }

  void process::run(){
    fiber = fibers::fiber([this](){ this-> base_loop(); });
  }

  void process::base_loop(){
    vm->self(shared_from_this());

    try{
      // printf("%s: Start\n", this->name().c_str());
      _inloop = true;
      this->loop();
      // printf("%s: End\n", this->name().c_str());
    } catch (opp::process_exit &e){
      if (e.code != 0){
        fprintf(stderr, "\n%s: Exit process. OPP Exception: %s.\n", this->name().c_str(), e.what());
        e.print_backtrace(this->name());
      }
      // fprintf(stderr, "%s: End Excp\n", this->name().c_str());
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
    // if (!running())
    //   throw opp::process_exit(shared_from_this(), 1);

    if (_debug){
      fprintf(stderr, "%s -> %s | %s\n", opp::self()->to_string().c_str(), to_string().c_str(), std::to_string(msg.type()).c_str());
    }

    inqueue.push(msg);
  }

  void process::stop(int code){
    // fprintf(stderr, "Stop %s %d\n", to_string().c_str(), code);
    vm->stop(shared_from_this(), code);
    // vm->send(exit_msg{shared_from_this(), code});
  }

  void process::monitor(){
    monitored_by.insert(self());
  }

  void process::demonitor(){
    monitored_by.insert(self());
  }

  std::any process::receive(
      const std::initializer_list<match_case> cases,
      const std::chrono::seconds &timeout){

    if (opp::self().get() != this){
      throw opp::exception("I can only receive on my process, not others.");
    }
    auto maxt = std::chrono::system_clock::now() + timeout;

    std::any msg;

    // First try all messages in queue.
    auto endI = messages.end();
    for (auto match: cases){
      for (auto I = messages.begin();I!=endI;++I){
        auto &msg = *I;
        if (match.test(msg)){
          auto msg = *I;
          messages.erase(I);
          return match.then(msg);
        }
      }
    }
    for (auto I = messages.begin();I!=endI;++I){
      maybe_exit_or_timeout(*I);
    }

    while(running()){
      // Now get a new message or timeout
      auto res = inqueue.pop_wait_until(msg, maxt);
      if (res == fibers::channel_op_status::timeout)
        msg = timeout_msg{shared_from_this()}; // FIXME? If timeout and not waiting for TO, exception.
      if (res != fibers::channel_op_status::success)
        throw_exit(1);

      // Try new message on all the cases
      for (auto match: cases){
        if (match.test(msg)){
          return match.then(msg);
        }
      }

      maybe_exit_or_timeout(msg);

      // If got here, maybe later wil be received. Now just push at the front the new value
      // This is nice as we use lists.
      messages.push_front(std::move(msg));
    }

    throw_exit(2); // impossible situation
    return false;
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
