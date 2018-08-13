#include <map>
#include <iostream>
#include "io.hpp"
#include "serverboards.hpp"


namespace Serverboards{
  struct PrivateData{
    std::map<std::string, std::function<json(const json&)>> method_map;
    bool running = false;
  };
  static PrivateData data;

  void rpc_method(std::string &&name, std::function<json(const json&)> &&func){
    data.method_map[name] = func;
  }

  void process_request(json &&req){
    popc::IO::stderr.println("parse");
  }

  void loop(){
    popc::IO::stderr.println("LOOP");
    data.running=true;
    while (data.running){
      auto line = popc::IO::stdin.readline();
      auto req = json::parse(line);
      process_request(std::move(req));
    };
  }
}
