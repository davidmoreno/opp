#include <iostream>
#include <unistd.h>
#include "process.hpp"
#include "opp.hpp"
#include "vm.hpp"

namespace opp{
  Symbol EXIT("exit");
  Symbol TIMEOUT("timeout");
  Symbol DOWN("down");

  std::chrono::seconds Process::FOREVER = std::chrono::hours(24*265*100);

  Process::Process(std::string &&name) : _name(name){
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
        fprintf(stderr, "%s: EXIT exception %s", this->name().c_str(), e.what());
      } catch (...) {
        fprintf(stderr, "%s: unknown exception", this->name().c_str());
      }
      _inloop = false;

      for(auto pr: monitored_by){
        pr->send(DOWN, {this});
      }
    }));

    vm->add_process(this);
  }

  Process::~Process(){
    // printf("%s: ~Process %p\n", _name.c_str(), this);
    exit();
    while(_inloop){ // FIXME spinning to get stop
    }
    vm->remove_process(this);
    thread.join();
  }

  void Process::send(const Symbol &s, std::any &&msg){
    if (!running())
      throw opp::process_exit();
    // printf("%s: Send %s\n", name().c_str(), s.name());
    std::unique_lock<std::mutex> lck(mtx);
    messages.push_back(std::make_pair(s, msg));
    message_signal.notify_all();
  }

  void Process::exit(){
    printf("%s: exit\n", _name.c_str());
    _running = false;
    send(EXIT, 0);
  }

  void Process::monitor(){
    monitored_by.insert(self());
  }

  void Process::demonitor(){
    monitored_by.insert(self());
  }

  Symbol Process::receive(const std::map<Symbol, std::function<void(const std::any &)>> &case_, const std::chrono::seconds &timeout){
    if (self() != this)
      throw bad_receiver();

    auto until = std::chrono::system_clock::now() + timeout;

    while(true){
      // printf("%s: receive 1\n", _name.c_str());
      std::unique_lock<std::mutex> lck(mtx);
      auto endI = messages.end();
      for(auto msg=messages.begin();msg!=endI;++msg){
        // printf("%s: Got message1\n", name().c_str());
        const Symbol &s = msg->first;
        auto it = case_.find(s);
        if (it == case_.end())
          continue; // Not in my case
        // printf("%s: Message for me: %s!\n", name().c_str(), msg->first.name());
        (it->second)(msg->second);

        messages.erase(msg);
        return it->first;
      }

      // printf("%s: Wait for message1\n", name().c_str());
      auto to_ = message_signal.wait_until(lck, until);
      if (to_ == std::cv_status::timeout)
        throw process_timeout();
      if (!running())
        throw process_exit();
    }
  }
  std::pair<Symbol, std::any> Process::receive(const std::set<Symbol> &symbols, const std::chrono::seconds &timeout){
    if (self() != this)
      throw bad_receiver();

    auto until = std::chrono::system_clock::now() + timeout;

    while(true){
      // printf("receive 2\n");
      std::unique_lock<std::mutex> lck(mtx);
      auto endI = messages.end();
      for(auto msg=messages.begin();msg!=endI;++msg){
        // printf("%s: Got message2\n", name().c_str());
        const Symbol &s = msg->first;
        auto it = symbols.find(s);
        if (it == symbols.end())
          continue; // Not in my case

        auto data = std::move(msg->second);

        messages.erase(msg);
        return std::make_pair(*it, data);
      }

      // printf("%s: Wait for message2\n", name().c_str());
      auto to_ = message_signal.wait_until(lck, until);
      if (to_ == std::cv_status::timeout)
        throw process_timeout();
      if (!running())
        throw process_exit();
    }
  }
  std::any Process::receive(Symbol symbol, const std::chrono::seconds &timeout){
    if (self() != this)
      throw bad_receiver();

    auto until = std::chrono::system_clock::now() + timeout;

    while(true){
      // printf("receive 3\n");
      std::unique_lock<std::mutex> lck(mtx);
      auto endI = messages.end();
      for(auto msg=messages.begin();msg!=endI;++msg){
        printf("%s: Got message3\n", name().c_str());
        const Symbol &s = msg->first;
        if (s == symbol){
          auto data = std::move(msg->second);
          messages.erase(msg);
          return data;
        }
        if (s == EXIT){
          throw opp::process_exit();
        }
        if (s == TIMEOUT){
          throw opp::process_timeout();
        }
      }

      // printf("%s: Wait for message3\n", name().c_str());
      auto to_ = message_signal.wait_until(lck, until);
      if (to_ == std::cv_status::timeout)
        throw process_timeout();
      if (!running())
        throw process_exit();
    }
  }
}
