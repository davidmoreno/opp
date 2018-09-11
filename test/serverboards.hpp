#pragma once

#include <string>
#include <functional>
#include "json.hpp"
using json = nlohmann::json;


namespace serverboards{
  void rpc_method(std::string &&name, std::function<json(const json&)> &&func);
  void loop();
}
