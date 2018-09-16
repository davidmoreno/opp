#include "opp.hpp"
#include "vm.hpp"

int main(){
  fprintf(stderr, "Start\n");
  opp::start();
  opp::vm->print_stats();
  fprintf(stderr, "Stop\n");
  opp::stop();
  fprintf(stderr, "The final End\n");
}
