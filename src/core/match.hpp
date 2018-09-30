#pragma once

#include <functional>
#include <any>
#include "reference.hpp"
#include "utils.hpp"

namespace opp{
  struct match_test;
  struct match_case;

  template<typename A>
  match_test match_type();

  struct match_case{
    std::function<bool(const std::any &)> test;
    std::function<std::any(std::any)> then;

    match_case(
        std::function<bool(const std::any &)> test_,
        std::function<std::any(std::any)> then_) : test(test_), then(then_
      ) { }

    match_case(match_test t);

    template<typename A>
    match_case(A fun){
      using TA = typename ::opp::utils::lambda_arg<A>::type;
      test = match_type<TA>().test;
      then = [fun](std::any v){
        fun(std::any_cast<TA>(std::move(v)));
        return std::any();
      };
    }

  };

  struct match_test{
    std::function<bool(const std::any &)> test;

    match_case then(std::function<std::any(std::any)> thenf){
      return match_case(std::move(test), std::move(thenf));
    }
  };

  template<typename A>
  match_test match_type(){
    return match_test{[](const std::any &a) -> bool{
      if (a.type() != typeid(A))
        return false;
      return true;
    }};
  }

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
