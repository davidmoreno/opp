#pragma once

#include <vector>
#include <map>

namespace opp::utils{

  template<typename TK, typename TV>
  std::vector<TK> extract_keys(std::map<TK, TV> const& input_map) {
    std::vector<TK> retval;
    for (auto const& element : input_map) {
      retval.push_back(element.first);
    }
    return retval;
  }

  template<typename TK, typename TV>
  std::vector<TV> extract_values(std::map<TK, TV> const& input_map) {
    std::vector<TV> retval;
    for (auto const& element : input_map) {
      retval.push_back(element.second);
    }
    return retval;
  }

  /// Can destructore the type of the argument of a lambda
  template <typename T>
  struct lambda_arg : public lambda_arg<decltype(&T::operator())>{};
  template <typename ClassType, typename ArgType>
  struct lambda_arg<void (ClassType::*) (ArgType args) const> {
      using type = ArgType;
  };

}
