class BlockingQueue{


public:
  add(opp::Symbol &&s, std::any &&);
  std::any &&find_or_wait_and_remove(std::set<std::symbol> &&ss, std::chrono::seconds &&timeout);
};
