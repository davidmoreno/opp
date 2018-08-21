#pragma once

#include <libgen.h>
#include <string.h>
#include "string.hpp"

#define OPP_DEBUG(...) opp::Logger::log(__FILE__, __LINE__, opp::Logger::DEBUG, __VA_ARGS__)
#define OPP_WARNING(...) opp::Logger::log(__FILE__, __LINE__, opp::Logger::WARNING, __VA_ARGS__)
#define OPP_ERROR(...) opp::Logger::log(__FILE__, __LINE__, opp::Logger::ERROR, __VA_ARGS__)
#define OPP_INFO(...) opp::Logger::log(__FILE__, __LINE__, opp::Logger::INFO, __VA_ARGS__)

namespace opp::Logger{
  class Logger;


  extern Logger *logger;

  enum LogLevel{
    DEBUG,
    WARNING,
    ERROR,
    INFO,
  };

  class Logger : public Process{
  public:
    Logger();
    ~Logger();

    virtual void loop();

    void log(const char *filename, int lineno, LogLevel loglevel, const std::string &msg);
  };

  template<typename... Args>
  static void log(const char *filename, int lineno, LogLevel loglevel, Args... args){
    auto str = concat(args...);
    if (!logger){
      char *filename2 = strdupa(filename);
      fprintf(::stderr, "%s:%d %s", basename(filename2), lineno, (str + "\n").c_str());
    } else {
      logger->log(filename, lineno, loglevel, str);
    }
  }
}
