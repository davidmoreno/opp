#include <iostream>
#include "process.hpp"
#include "popc.hpp"

namespace popc{

  Symbol EXIT;
  Symbol TIMEOUT;

  Process::Process(){
    popc::start_process(this);
  }
  Process::Process(std::string &&_name) : name(_name){
    popc::start_process(this);
  }

  Process::~Process(){

  }

  void Process::send(const Symbol &s, std::any &&msg){
    printf("%s: Send %s\n", name.c_str(), s.name());
    std::unique_lock<std::mutex> lck(mtx);
    messages.push_back(std::make_pair(s, msg));
    newmessage.notify_all();
  }

  Symbol Process::receive(const std::map<Symbol, std::function<void(const std::any &)>> &case_, const std::chrono::seconds &timeout){
    while(true){
      printf("%s: Wait for message1\n", name.c_str());
      std::unique_lock<std::mutex> lck(mtx);
      newmessage.wait(lck);
      if (!running)
        return EXIT;

      auto endI = messages.end();
      for(auto msg=messages.begin();msg!=endI;++msg){
        printf("%s: Got message1\n", name.c_str());
        const Symbol &s = msg->first;
        auto it = case_.find(s);
        if (it == case_.end())
          continue; // Not in my case
        (it->second)(msg->second);

        messages.erase(msg);
        return it->first;
      }
    }
  }
  std::pair<Symbol, std::any> Process::receive(const std::set<Symbol> &symbols, const std::chrono::seconds &timeout){
    while(true){
      printf("%s: Wait for message2\n", name.c_str());
      std::unique_lock<std::mutex> lck(mtx);
      newmessage.wait(lck);
      if (!running)
        return std::make_pair(EXIT, std::any(0));

      auto endI = messages.end();
      for(auto msg=messages.begin();msg!=endI;++msg){
        printf("%s: Got message2\n", name.c_str());
        const Symbol &s = msg->first;
        auto it = symbols.find(s);
        if (it == symbols.end())
          continue; // Not in my case

        auto data = std::move(msg->second);

        messages.erase(msg);
        return std::make_pair(*it, data);
      }
    }
  }
  std::any Process::receive(Symbol symbol, const std::chrono::seconds &timeout){
    while(true){
      printf("%s: Wait for message3\n", name.c_str());
      std::unique_lock<std::mutex> lck(mtx);
      newmessage.wait(lck);
      if (!running)
        return EXIT;

      auto endI = messages.end();
      for(auto msg=messages.begin();msg!=endI;++msg){
        printf("%s: Got message3\n", name.c_str());
        const Symbol &s = msg->first;
        if (s == symbol){
          auto data = std::move(msg->second);
          messages.erase(msg);
          return data;
        }
      }
    }
  }
}
