#include "process.hpp"

namespace popc{

  Symbol EXIT;
  Symbol TIMEOUT;

  Process::Process(){

  }

  Process::~Process(){

  }

  void Process::send(const Symbol &s, std::any &&msg){

  }

  std::pair<Symbol, std::any> Process::receive(const std::map<Symbol, std::function<void(const std::any &)>> &case_, const std::chrono::seconds &timeout){
    return std::make_pair(EXIT, 0);
  }
  std::pair<Symbol, std::any> Process::receive(const std::set<Symbol> &symbols, const std::chrono::seconds &timeout){
    return std::make_pair(EXIT, 0);
  }
  std::any Process::receive(Symbol, const std::chrono::seconds &timeout){
    return 0;
  }
}
