#include "match.hpp"

opp::match_case::match_case(match_test t) : test(t.test){
  then = [](std::any msg){ return msg; };
}
