#pragma once

#include <exception>

namespace opp{
  class process;

  class exception : public std::exception {};

  class bad_receiver : public opp::exception {
  public:
    const char *what() const noexcept{
      return "Receive on wrong process. Only currently executing process can call receive.";
    }
  };

  class process_exception : public opp::exception {
  public:
    opp::process *process;
    process_exception(opp::process *pr) : process(pr){};
    const char *what() const noexcept{
      return "Receive on wrong process. Only currently executing process can call receive.";
    }
  };

  class process_timeout : public opp::process_exception {
  public:
    process_timeout(opp::process *pr) : process_exception(pr){};
    const char *what() const noexcept{
      return "timeout";
    }
  };

  class process_exit : public opp::process_exception {
  public:
    process_exit(opp::process *pr) : process_exception(pr){};
    const char *what() const noexcept{
      return "exit";
    }
  };

  class not_implemented : public opp::exception {
  public:
    const char *what() const noexcept{
      return "not_implemented";
    }
  };

  class not_initialized : public opp::exception {
  public:
    const char *what() const noexcept{
      return "not_initialized";
    }
  };

  class already_initialized : public opp::exception {
  public:
    const char *what() const noexcept{
      return "already_initialized";
    }
  };

  void start();
  void stop();
  process *self();
}
