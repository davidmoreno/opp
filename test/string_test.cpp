#include <core/string.hpp>
#include <logger.hpp>
#include <core/exceptions.hpp>
#include <fmt/format.h>

#define OPP_ASSERT(v) { if (!v){ OPP_ERROR("Not true"); throw opp::exception("Not true");} }

int main(){
  std::string test = "/start/";

  std::string_view test_view = test;
  std::string test_to = "/start/here";
  std::string_view test_to_view = "/start/here";

  std::string_view test_to_view_f = "/star/here";
  std::string_view empty = "";


  OPP_ASSERT(true == opp::string::startswith(test_to, test_view));
  OPP_ASSERT(true == opp::string::startswith(test_to, test_view));

  OPP_ASSERT(true == opp::string::startswith(test_to_view, test_view));
  OPP_ASSERT(true == opp::string::startswith(test_to_view, test_view));
  OPP_ASSERT(true == opp::string::startswith(test_to_view, test_view));
  OPP_ASSERT(true == opp::string::startswith(test_to_view, test_view));
  OPP_ASSERT(true == opp::string::startswith(test_to_view, test_view));
  OPP_ASSERT(true == opp::string::startswith(test_to_view, test_view));
  OPP_ASSERT(true == opp::string::startswith(test_to_view, test_view));
  OPP_ASSERT(true == opp::string::startswith(test_to_view, test_view));
  OPP_ASSERT(true == opp::string::startswith(test_to_view, test_view));
  OPP_ASSERT(true == opp::string::startswith(test_to_view, test_view));
  OPP_ASSERT(true == opp::string::startswith(test_to_view, test_view));
  OPP_ASSERT(true == opp::string::startswith(test_to_view, test_view));

  OPP_ASSERT(false == opp::string::startswith(test_to_view_f, test_view));
  OPP_ASSERT(false == opp::string::startswith(test_to_view_f, test_view));

  OPP_ASSERT(true == opp::string::startswith(test_to_view, test_view));
  OPP_ASSERT(true == opp::string::startswith(test_to_view, test_view));

  OPP_ASSERT(true == opp::string::startswith(test_to_view, empty));
  OPP_ASSERT(true == opp::string::startswith(test_to_view, empty));
  OPP_ASSERT(true == opp::string::startswith(test_to_view, empty));
  OPP_ASSERT(true == opp::string::startswith(test_to_view, empty));


}
