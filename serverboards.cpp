#include <map>
#include <iostream>
#include "io.hpp"
#include "term.hpp"
#include "vm.hpp"
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
    opp::IO::stderr->println("process: " + req.dump());

    auto method = req.find("method");
    if (method == req.end()){
      opp::IO::stderr->println("No method. Maybe response.");
      return;
    }
    auto func = data.method_map.find(*method);
    if (func == data.method_map.end()){
      json ret = {{"id", req.at("id")}, {"error", "not_found"}};
      opp::IO::stdout->println(opp::Term::color(ret.dump(), opp::Term::GREEN));
      return;
    }
    try{
      auto result = func->second(req.at("params"));
      json ret = {{"id", req["id"]}, {"result", result}};
      opp::IO::stdout->println(ret.dump());
    } catch (const std::exception &e){
      json ret = {{"id", req["id"]}, {"error", e.what()}};
      opp::IO::stdout->println(ret.dump());
    }

  }

  void loop(){
    opp::IO::stderr->println("LOOP");
    data.running=true;
    while (data.running){
      try{
        auto line = opp::IO::stdin->readline();
        auto req = json::parse(line);
        process_request(std::move(req));

        opp::vm->print_stats();
        // opp::IO::stderr->println("Debug STOP");
        // data.running=false; // To stop on debug
      } catch (std::exception &e){
        fprintf(stderr, "Exception at Serverboards::loop: %s.\n", e.what());
        exit(1);
      }
    };
  }
}
