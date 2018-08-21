#include <map>
#include <iostream>
#include "io.hpp"
#include "logger.hpp"
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
    OPP_DEBUG("process: ", req.dump());

    auto method = req.find("method");
    if (method == req.end()){
      OPP_ERROR("No method?? ", req.dump());
      return;
    }
    auto func = data.method_map.find(*method);
    if (func == data.method_map.end()){
      json ret = {{"id", req.at("id")}, {"error", "not_found"}};
      OPP_DEBUG(ret.dump());
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
    opp::Logger::Logger logger;

    data.running=true;
    OPP_DEBUG("Starting loop");
    while (data.running){
      try{
        auto line = opp::IO::stdin->readline();
        auto req = json::parse(line);
        process_request(std::move(req));

        opp::vm->print_stats();
        // opp::IO::stderr->println("Debug STOP");
        // data.running=false; // To stop on debug
      } catch (opp::process_exit &){
        OPP_DEBUG("Exit.")
        return;
      } catch (std::exception &e){
        fprintf(stderr, "Exception at Serverboards::loop: %s.\n", e.what());
        exit(1);
      }
    };
  }
}
