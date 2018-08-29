#include "term.hpp"
#include "io.hpp"
#include "logger.hpp"

namespace opp::logger{
  std::shared_ptr<logger> __logger = nullptr;

  struct log_msg{
    int pid;
    const char *filename;
    int lineno;
    LogLevel loglevel;
    std::string message;
  };
  struct flush_msg{
    opp::process *process;
  };
  struct flush_ready_msg{};

  logger::logger(){
  }
  logger::~logger(){
    __logger = nullptr;
  }

  void logger::run(){
    __logger = this->shared_from_this();
    process::run();
  }

  void logger::loop(){
    while(running()){
      receive<log_msg, flush_msg>(
        std::function<void(const log_msg &)>([](const log_msg &msg){
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

          opp::io::stderr->println(term::color(
            opp::concat(
              "#", msg.pid, " ",
              "[", timestamp, "] ",
              "[", filename, ":", msg.lineno, "]\t"
            ), color),
            msg.message
          ); //term::color("msg"), term::RED);
        }),
        std::function<void(const flush_msg&)>([](const flush_msg &pr){
          pr.process->send(flush_ready_msg{});
        }),
        opp::process::FOREVER
      );
    }
  }

  void logger::log(const char *filename, int lineno, LogLevel loglevel, const std::string &msg){
    send(log_msg{self()->pid(), filename, lineno, loglevel, msg});
  }
  /// Blocks until this is processed
  void logger::flush(){
    send(flush_msg{this});
    receive<flush_ready_msg>();
  }
}
