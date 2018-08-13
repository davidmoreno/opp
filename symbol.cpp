#include "symbol.hpp"

// Just a counter
static int symbol_id = 1;

popc::Symbol::Symbol(){
  id = symbol_id++;
}
