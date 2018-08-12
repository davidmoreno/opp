#include "process.hpp"

namespace SPC{

  Process::Process(){

  }

  Process::~Process(){

  }

  void Process::send(const Symbol &s, std::any &&msg){

  }

  std::pair<Symbol, std::any> Process::receive(const std::map<Symbol, std::function<void(const std::any &)>> &case_, const std::chrono::seconds &timeout){

  }
  std::pair<Symbol, std::any> Process::receive(const std::set<Symbol> &symbols, const std::chrono::seconds &timeout){

  }
  std::any Process::receive(Symbol, const std::chrono::seconds &timeout){

  }
}
