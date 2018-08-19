#pragma once

#include <exception>

namespace opp{
  class Process;

  class exception : public std::exception {};

  class bad_receiver : public opp::exception {
    const char *what() const noexcept{
      return "Receive on wrong process. Only currently executing process can call receive.";
    }
  };

  class process_timeout : public opp::exception {
    const char *what() const noexcept{
      return "timeout";
    }
  };

  class process_exit : public opp::exception {
    const char *what() const noexcept{
      return "exit";
    }
  };

  class not_implemented : public opp::exception {
    const char *what() const noexcept{
      return "not_implemented";
    }
  };

  class not_initialized : public opp::exception {
    const char *what() const noexcept{
      return "not_initialized";
    }
  };

  class already_initialized : public opp::exception {
    const char *what() const noexcept{
      return "already_initialized";
    }
  };

  void start();
  void stop();
  Process *self();
}
