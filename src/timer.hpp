#pragma once

#include <boost/fiber/operations.hpp>
#include <chrono>

namespace opp{
  namespace timer{
    void sleep(uint32_t seconds){
      boost::this_fiber::sleep_for(std::chrono::seconds(seconds));
    }
    void sleep(std::chrono::seconds s){
      boost::this_fiber::sleep_for(s);
    }
  }
}
