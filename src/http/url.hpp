#pragma once

#include <functional>
#include <variant>
#include <memory>
#include "string.hpp"
#include "request.hpp"
#include "response.hpp"
#include "core/exceptions.hpp"

namespace opp::http{
  class url{
  public:
    using handler_f = std::function<response(const request &)>;
    using url_v = std::vector<url>;

    std::string path;
    std::variant<handler_f, url_v> next;

    url(std::initializer_list<url> urls_) : path(""), next(std::vector(urls_)) {};
    url(url_v urls) : path(""), next(urls) {}
    url(handler_f f) : path(""), next(f) {}
    url(std::string path_, url_v urls_) : path(path_), next(urls_) {}
    url(std::string path_, handler_f f) : path(path_), next(f) {}
    url(std::pair<std::string, handler_f> pair) : path(pair.first), next(pair.second) {}
    url(std::pair<std::string, url_v> pair) : path(pair.first), next(pair.second) {}

    std::optional<handler_f> find(std::string_view request_path){
      if (::opp::string::startswith(request_path, path)){
        if (std::holds_alternative<url_v>(next)){
          for (auto &url: std::get<url_v>(next)){
            auto res = url.find(request_path.substr(path.size(), 10000));
            if (res)
              return res;
          }
        }
        if (std::holds_alternative<handler_f>(next)){
          return std::get<handler_f>(next);
        }
      }
      return std::nullopt;
    }
  };
}
