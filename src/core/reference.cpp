#include <sstream>
#include "reference.hpp"

// Just a counter
static int reference_id = 1;

opp::reference::reference(){
  _id = reference_id++;
}

std::string opp::reference::to_string(){
  std::stringstream ss;
  ss<<"[#";
  ss<<_id;
  ss<<"]";
  return ss.str();
}
