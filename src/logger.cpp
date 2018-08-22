#include "term.hpp"
#include "io.hpp"
#include "logger.hpp"

namespace opp::logger{
  logger *__logger = nullptr;
  static symbol LOG("log");
  static symbol FLUSH("flush");
  static symbol FLUSH_READY("flush_ready");

  struct LogMessage{
    const char *filename;
    int lineno;
    LogLevel loglevel;
    std::string message;
  };

  logger::logger(){
    __logger = this;
  }
  logger::~logger(){
    __logger = nullptr;
  }

  void logger::loop(){
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

          auto color = term::WHITE;
          switch(msg.loglevel){
            case DEBUG:
              color = term::BLUE;
            break;
            case WARNING:
              color = term::YELLOW;
            break;
            case ERROR:
              color = term::RED;
            break;
            case INFO:
              color = term::WHITE;
            break;
          }

          opp::io::stderr->println(term::color(
            opp::concat(
              "[", timestamp, "] ",
              "[", filename, ":", msg.lineno, "]\t"
            ), color),
            msg.message
          ); //term::color("msg"), term::RED);
        }},
        {FLUSH, [](const std::any &pr){
          auto process = std::any_cast<opp::process*>(pr);
          process->send(FLUSH_READY, {});
        }}
      });
    }
  }

  void logger::log(const char *filename, int lineno, LogLevel loglevel, const std::string &msg){
    send(LOG, LogMessage{filename, lineno, loglevel, msg});
  }
  /// Blocks until this is processed
  void logger::flush(){
    send(FLUSH, this);
    receive(FLUSH_READY);
  }
}
