#include "symbol.hpp"

// Just a counter
static int symbol_id = 1;

opp::symbol::symbol(){
  _id = symbol_id++;
}
opp::symbol::symbol(const char *name) : _name(name){
  _id = symbol_id++;
}

std::ostream& opp::operator<< (std::ostream& os, const opp::symbol &sym){
  const char *name = sym.name();
  if (name){
    os << sym.name();
  } else {
    os << "symbol["<<sym.id()<<"]";
  }

  return os;
}
