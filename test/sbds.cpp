#include "opp.hpp"
#include "serverboards.hpp"

int main(int argc, char **argv){
  opp::start();

  serverboards::rpc_method("echo", [](const json &args){
    return args;
  });

  serverboards::loop();

  printf("EXIT!! end!!\n");
}
