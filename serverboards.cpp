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
    opp::IO::stderr->println("parse: " + req.dump());
  }

  void loop(){
    opp::IO::stderr->println("LOOP");
    data.running=true;
    try{
      while (data.running){
        auto line = opp::IO::stdin->readline();
        auto req = json::parse(line);
        process_request(std::move(req));

        // opp::IO::stderr->println("Debug STOP");
        // data.running=false; // To stop on debug
      };
    } catch (std::exception &e){
      printf("Exception exit: %s\n", e.what());
    }
  }
}
