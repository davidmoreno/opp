#include "symbol.hpp"

// Just a counter
static int symbol_id = 1;

opp::Symbol::Symbol(){
  _id = symbol_id++;
}
opp::Symbol::Symbol(const char *name) : _name(name){
  _id = symbol_id++;
}

std::ostream& opp::operator<< (std::ostream& os, const opp::Symbol &sym){
  const char *name = sym.name();
  if (name){
    os << sym.name();
  } else {
    os << "Symbol["<<sym.id()<<"]";
  }

  return os;
}
