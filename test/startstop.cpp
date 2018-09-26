#include "opp.hpp"
#include "vm.hpp"

int main(){
  fprintf(stderr, "Start\n");
  fprintf(stderr, "Count %ld\n", opp::vm.use_count());
  opp::start();
  fprintf(stderr, "Count %ld\n", opp::vm.use_count());
  opp::vm->print_stats();
  fprintf(stderr, "Stop %ld\n", opp::vm.use_count());
  opp::stop();
  auto scheduler = boost::fibers::context::active()->get_scheduler();
  fprintf(stderr, "The final End. %s\n", std::to_string(typeid(scheduler)).c_str());
}
