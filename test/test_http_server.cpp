#include <string>
#include <vector>
#include <fmt/format.h>
#include "opp.hpp"
#include "vm.hpp"
#include "http/server.hpp"
#include "string.hpp"
#include "logger.hpp"

int main(int argc, char **argv){
  opp::start();

  auto server = opp::start<opp::http::server>(argv[1], argv[2]);

  server->set_root_url({
    {"/", [](const opp::http::request &){ return opp::http::response("hello"); }},
    {"/stats", [](const opp::http::request &){ return opp::http::response("stats"); }},
  });

  OPP_DEBUG("Im ready.");

  boost::this_fiber::sleep_for(std::chrono::seconds(100000));

  opp::stop();
}
