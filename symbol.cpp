#include "symbol.hpp"

// Just a counter
static int symbol_id = 1;

popc::Symbol::Symbol(){
  _id = symbol_id++;
}
popc::Symbol::Symbol(const char *name) : _name(name){
  _id = symbol_id++;
}

std::ostream& popc::operator<< (std::ostream& os, const popc::Symbol &sym){
  const char *name = sym.name();
  if (name){
    os << sym.name();
  } else {
    os << "Symbol["<<sym.id()<<"]";
  }

  return os;
}
