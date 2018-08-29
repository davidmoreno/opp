#include <map>
#include <iostream>
#include "io.hpp"
#include "logger.hpp"
#include "term.hpp"
#include "vm.hpp"
#include "serverboards.hpp"
#include "utils.hpp"
#include "task.hpp"


namespace serverboards{
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
      opp::io::stdout->println(ret.dump());
      return;
    }
    // Starts as new task, to do not block here.
    opp::task::start([func, req]{
      try{
        auto result = func->second(req.at("params"));
        json ret = {{"id", req["id"]}, {"result", result}};
        opp::io::stdout->println(ret.dump());
      } catch (const std::exception &e){
        json ret = {{"id", req["id"]}, {"error", e.what()}};
        opp::io::stdout->println(ret.dump());
      }
    });

  }

  void loop(){
    auto logger = std::make_shared<opp::logger::logger>();
    logger->run();

    rpc_method("dir", [](const json &) -> json{
      return opp::utils::extract_keys(data.method_map);
    });

    data.running=true;
    OPP_DEBUG("Starting loop");
    while (data.running){
      try{
        auto line = opp::io::stdin->readline();
        auto req = json::parse(line);
        process_request(std::move(req));

        opp::vm->print_stats();
        // opp::io::stderr->println("Debug STOP");
        // data.running=false; // To stop on debug
      } catch (opp::process_exit &){
        OPP_INFO("Exit.");
        logger->flush();
        exit(0);
      } catch (std::exception &e){
        fprintf(stderr, "Exception at serverboards::loop: %s.\n", e.what());
        exit(1);
      }
    };
  }
}
