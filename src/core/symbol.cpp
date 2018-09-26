#include <sstream>
#include "symbol.hpp"

// Just a counter
static int symbol_id = 1;

opp::symbol::symbol(){
  _id = symbol_id++;
  _name = "unknown";
}
opp::symbol::symbol(const char *name) : _name(name){
  _id = symbol_id++;
}

std::string opp::symbol::to_string(){
  std::stringstream ss;
  ss<<"symbol[";
  if (_name)
    ss<<_name;
  else
    ss<<_id;
  ss<<"]";
  return ss.str();
}
