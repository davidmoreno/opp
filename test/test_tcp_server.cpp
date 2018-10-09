#include <string>
#include <vector>
#include <fmt/format.h>
#include "io/tcp/server.hpp"
#include "io/tcp/peer.hpp"
#include "string.hpp"
#include "logger.hpp"
#include "vm.hpp"
#include "task.hpp"

int main(int argc, char **argv){
  opp::start();

  if (argc!=3){
    OPP_ERROR("Requires 3 arguments");
    opp::stop();
    return 1;
  }

  auto server = opp::start<opp::io::tcp::server>(argv[1], argv[2]);
  // client->set_debug(true);
  // opp::self()->set_debug(true);

  while(opp::vm->running()){
    auto peer = server->wait_peer();
    // OPP_INFO("New peer {}", std::to_string(peer));

    opp::task::start([peer]{
      opp::io::buffer_t data(1024);
      peer->read(data);
      OPP_INFO("Got {}", std::to_string(data));
      peer->stop();
    });
  }
  server->stop();

  OPP_INFO("EOF");
  opp::stop();
}
