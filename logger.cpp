#include "term.hpp"
#include "io.hpp"
#include "logger.hpp"

namespace opp::Logger{
  Logger *logger = nullptr;
  static Symbol LOG("log");
  static Symbol FLUSH("flush");
  static Symbol FLUSH_READY("flush_ready");

  struct LogMessage{
    const char *filename;
    int lineno;
    LogLevel loglevel;
    std::string message;
  };

  Logger::Logger(){
    logger = this;
  }
  Logger::~Logger(){
    logger = nullptr;
  }

  void Logger::loop(){
    while(running()){
      receive({
        {LOG, [](const std::any &message){
          // from https://stackoverflow.com/questions/9527960/how-do-i-construct-an-iso-8601-datetime-in-c
          time_t now;
          time(&now);
          char timestamp[sizeof "2011-10-08T07:07:09Z"];
          strftime(timestamp, sizeof timestamp, "%FT%TZ", gmtime(&now));


          auto msg = std::any_cast<LogMessage>(message);
          auto filename = basename(strdupa(msg.filename));

          auto color = Term::WHITE;
          switch(msg.loglevel){
            case DEBUG:
              color = Term::BLUE;
            break;
            case WARNING:
              color = Term::YELLOW;
            break;
            case ERROR:
              color = Term::RED;
            break;
            case INFO:
              color = Term::WHITE;
            break;
          }

          opp::IO::stderr->println(Term::color(
            opp::concat(
              "[", timestamp, "] ",
              "[", filename, ":", msg.lineno, "]\t"
            ), color),
            msg.message
          ); //Term::color("msg"), Term::RED);
        }},
        {FLUSH, [](const std::any &pr){
          auto process = std::any_cast<Process*>(pr);
          process->send(FLUSH_READY, {});
        }}
      });
    }
  }

  void Logger::log(const char *filename, int lineno, LogLevel loglevel, const std::string &msg){
    send(LOG, LogMessage{filename, lineno, loglevel, msg});
  }
  /// Blocks until this is processed
  void Logger::flush(){
    send(FLUSH, this);
    receive(FLUSH_READY);
  }
}
