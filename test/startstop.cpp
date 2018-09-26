#include "opp.hpp"
#include "vm.hpp"

int main(){
  fprintf(stderr, "Main Start\n");
  opp::start();
  opp::vm->print_stats();
  opp::stop();
  fprintf(stderr, "Main Stop, %ld uses of vm.\n", opp::vm.use_count());
}
