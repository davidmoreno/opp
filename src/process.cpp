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

  symbol process::receive(const std::map<symbol, std::function<void(const std::any &)>> &case_, const std::chrono::seconds &timeout){
    if (self() != this)
      throw bad_receiver();

    auto until = std::chrono::system_clock::now() + timeout;

    while(true){
      // printf("%s: receive 1\n", _name.c_str());
      std::unique_lock<std::mutex> lck(mtx);
      auto endI = messages.end();
      for(auto msg=messages.begin();msg!=endI;++msg){
        // printf("%s: Got message1\n", name().c_str());
        const symbol &s = msg->first;
        auto it = case_.find(s);
        if (it == case_.end()){
          maybe_exit_or_timeout(s, msg->second);
          continue; // Not in my case
        }
        // printf("%s: Message for me: %s!\n", name().c_str(), msg->first.name());
        (it->second)(msg->second);

        messages.erase(msg);
        return it->first;
      }

      // printf("%s: Wait for message1\n", name().c_str());
      auto to_ = message_signal.wait_until(lck, until);
      if (to_ == std::cv_status::timeout)
        throw process_timeout(this);
      if (!running())
        throw process_exit(this);
    }
  }
  std::pair<symbol, std::any> process::receive(const std::set<symbol> &symbols, const std::chrono::seconds &timeout){
    if (self() != this)
      throw bad_receiver();

    auto until = std::chrono::system_clock::now() + timeout;

    while(true){
      // printf("receive 2\n");
      std::unique_lock<std::mutex> lck(mtx);
      auto endI = messages.end();
      for(auto msg=messages.begin();msg!=endI;++msg){
        // printf("%s: Got message2\n", name().c_str());
        const symbol &s = msg->first;
        auto it = symbols.find(s);
        if (it == symbols.end()){
          maybe_exit_or_timeout(s, msg->second);
          continue; // Not in my case
        }

        auto data = std::move(msg->second);

        messages.erase(msg);
        return std::make_pair(*it, data);
      }

      // printf("%s: Wait for message2\n", name().c_str());
      auto to_ = message_signal.wait_until(lck, until);
      if (to_ == std::cv_status::timeout)
        throw process_timeout(this);
      if (!running())
        throw process_exit(this);
    }
  }
  std::any process::receive(opp::symbol symbol, const std::chrono::seconds &timeout){
    if (self() != this)
      throw bad_receiver();

    auto until = std::chrono::system_clock::now() + timeout;

    while(true){
      // printf("receive 3\n");
      std::unique_lock<std::mutex> lck(mtx);
      auto endI = messages.end();
      for(auto msg=messages.begin();msg!=endI;++msg){
        const opp::symbol &s = msg->first;
        if (s == symbol){
          auto data = std::move(msg->second);
          messages.erase(msg);
          return data;
        }
        maybe_exit_or_timeout(s, msg->second);
      }

      // printf("%s: Wait for message3\n", name().c_str());
      auto to_ = message_signal.wait_until(lck, until);
      if (to_ == std::cv_status::timeout)
        throw process_timeout(this);
      if (!running())
        throw process_exit(this);
    }
  }
}
