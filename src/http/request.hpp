#pragma once

#include <string>
#include <vector>
#include <unordered_map>

namespace opp::io{
  class buffer_t;
}

namespace opp::http{
  class request{
  public:
    std::string_view method;
    std::string_view path;
    std::unordered_map<std::string_view, std::string_view> headers;

    request();
    bool parse(const opp::io::buffer_t &data);
    void clean();
  };
}
