#include <opp.hpp>
#include <logger.hpp>
#include <task.hpp>
#include <timer.hpp>


int main(){
  opp::start();

  for (auto i=0;i<10;i++){
    opp::task::start([i]{
      OPP_DEBUG("Sleep {}", i);
      opp::timer::sleep(1);
      OPP_DEBUG("Woke up {}", i);
    });
  }
  opp::timer::sleep(10);

  opp::stop();
}
