#pragma once

#include <functional>
#include <any>
#include "reference.hpp"

namespace opp{
  struct match_case{
    std::function<bool(const std::any &)> test;
    std::function<std::any(std::any)> then;
  };

  struct match_test{
    std::function<bool(const std::any &)> test;

    match_case then(std::function<std::any(std::any)> thenf){
      return match_case{std::move(test), std::move(thenf)};
    }
  };

  template<typename A>
  match_test match_ref(reference ref){
    return match_test{[ref](const std::any &a) -> bool{
      if (a.type() != typeid(A))
        return false;
      if (std::any_cast<A>(a).ref != ref)
        return false;
      return true;
    }};
  }
}
