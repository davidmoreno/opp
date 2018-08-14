#pragma once

#include <exception>

namespace popc{
  class Process;

  class exception : public std::exception {};

  class bad_receiver : public popc::exception {
    const char *what() const noexcept{
      return "Receive on wrong process. Only currently executing process can call receive.";
    }
  };

  class process_timeout : public popc::exception {
    const char *what() const noexcept{
      return "timeout";
    }
  };

  class process_exit : public popc::exception {
    const char *what() const noexcept{
      return "exit";
    }
  };

  class not_implemented : public popc::exception {
    const char *what() const noexcept{
      return "not_implemented";
    }
  };

  class not_initialized : public popc::exception {
    const char *what() const noexcept{
      return "not_initialized";
    }
  };

  class already_initialized : public popc::exception {
    const char *what() const noexcept{
      return "already_initialized";
    }
  };

  void start();
  void stop();
  Process *self();
  void start_process(Process *pr);
  void stop_process(Process *pr);
}
