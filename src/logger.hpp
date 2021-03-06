#pragma once

#include <libgen.h>
#include <string.h>
#include <memory.h>
#include <fmt/format.h>
#include "core/process.hpp"
#include "core/string.hpp"

#define OPP_DEBUG(...) opp::logger::log(__FILE__, __LINE__, opp::logger::DEBUG, __VA_ARGS__)
#define OPP_WARNING(...) opp::logger::log(__FILE__, __LINE__, opp::logger::WARNING, __VA_ARGS__)
#define OPP_ERROR(...) opp::logger::log(__FILE__, __LINE__, opp::logger::ERROR, __VA_ARGS__)
#define OPP_INFO(...) opp::logger::log(__FILE__, __LINE__, opp::logger::INFO, __VA_ARGS__)

namespace opp{
  extern thread_local uint32_t tid;
}

namespace opp::logger{
  class logger;

  extern std::shared_ptr<logger> __logger;

  enum LogLevel{
    DEBUG,
    WARNING,
    ERROR,
    INFO,
  };

  class logger : public process{
  public:
    logger();
    ~logger();

    virtual void loop();

    std::shared_ptr<logger> shared_from_this(){
        return std::static_pointer_cast<logger>(process::shared_from_this());
    }

    void log(const char *filename, int lineno, LogLevel loglevel, const std::string &msg);
    void flush();
  };

  template<typename... Args>
  inline void log(const char *filename, int lineno, LogLevel loglevel, Args... args){
    auto str = fmt::format(args...);
    if (!__logger){
      char *filename2 = strdupa(filename);
      fprintf(::stderr, "[%d] %s:%d %s", opp::tid, basename(filename2), lineno, (str + "\n").c_str());
    } else {
      __logger->log(filename, lineno, loglevel, str);
    }
  }

  inline void flush(){
    auto myl = __logger;
    myl->flush();
  }
}
