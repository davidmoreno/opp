#include "core/term.hpp"
#include "io/file.hpp"
#include "logger.hpp"

namespace opp::logger{
  std::shared_ptr<logger> __logger = nullptr;

  struct log_msg{
    std::shared_ptr<opp::process> pid;
    const char *filename;
    int lineno;
    LogLevel loglevel;
    std::string message;
  };
  struct flush_msg{
    opp::process *process;
  };
  struct flush_ready_msg{};

  logger::logger() : process("logger") {
    if (__logger)
      throw opp::already_initialized();
  }
  logger::~logger(){
    __logger = nullptr;
  }

  void logger::loop(){
    __logger = this->shared_from_this();
    while(running()){
      receive({
        [](log_msg msg){
          // from https://stackoverflow.com/questions/9527960/how-do-i-construct-an-iso-8601-datetime-in-c
          time_t now;
          time(&now);
          char timestamp[sizeof "2011-10-08T07:07:09Z"];
          strftime(timestamp, sizeof timestamp, "%FT%TZ", gmtime(&now));

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

          std::string _name = msg.pid->to_string();

          opp::io::stderr->println("{} {}", term::color(
            fmt::format("{:<24} [{}] [{}:{}]",
              _name, timestamp, filename, msg.lineno
            ), color),
            msg.message
          ); //term::color("msg"), term::RED);
        },
        [](flush_msg pr){
          pr.process->send(flush_ready_msg{});
        }
      },
        opp::process::FOREVER
      );
    }
    fprintf(stderr, "Logger out\n");
  }

  void logger::log(const char *filename, int lineno, LogLevel loglevel, const std::string &msg){
    send(log_msg{self(), filename, lineno, loglevel, msg});
  }
  /// Blocks until this is processed
  void logger::flush(){
    send(flush_msg{this});
    self()->receive({
      match_type<flush_ready_msg>()
    });
  }
}
