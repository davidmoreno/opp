#include "vm.hpp"
#include "serverboards.hpp"

int main(int argc, char **argv){
  auto vm = popc::VM();

  Serverboards::rpc_method("echo", [](const json &args){
    return args;
  });

  Serverboards::loop();

  printf("EXIT!! end!!\n");
}
