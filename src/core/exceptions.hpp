#pragma once

#include <exception>
#include <memory>
#include <execinfo.h>
#include <cxxabi.h>
#include <fmt/format.h>
#include "process.hpp"
#include "string.hpp"
#include "opp.hpp"

namespace opp{
  class exception : public std::exception {
    void *trace[16];
    size_t trace_size;
  protected:
    std::string msg;
  public:
    exception(std::string msg = ""){
      this->msg = std::move(msg);
      trace_size = backtrace(trace, 16);
    }
    void print_backtrace(std::string name="??"){
      opp::print_backtrace(std::move(name), trace, trace_size);
    }
    const char *what() const noexcept{
      return msg.c_str();
    }
  };

  class process_exception : public opp::exception {
  public:
    process_t process;
    process_exception(process_t pr) : process(pr){
      msg = fmt::format(
        "Receive on wrong process. Only currently executing process can call receive. (receive at {}, should have been at {})",
        process->to_string(), self()->to_string()
      );
    };
  };

  class bad_cast : public opp::exception {
  public:
    bad_cast(std::string a, std::string b){
      char *adm = abi::__cxa_demangle(a.c_str(), nullptr, nullptr, nullptr);
      char *bdm = abi::__cxa_demangle(b.c_str(), nullptr, nullptr, nullptr);
      msg = fmt::format(
        "Bad cast. From {} to {}",
        adm ? adm : a,
        bdm ? bdm : b
      );
    }
  };

  class bad_receiver : public opp::process_exception {
  public:
    bad_receiver(process_t pr) : process_exception(pr){
      // print_backtrace();
      msg = fmt::format("bad receiver. Should be {}", to_string(pr));
    };
  };

  class process_timeout : public opp::process_exception {
  public:
    process_timeout(process_t pr) : process_exception(pr){
      msg = fmt::format("timeout {}", to_string(pr));
    };
  };

  class process_exit : public opp::process_exception {
  public:
    int code; // 0 controlled
    process_exit(process_t pr, int _code) : process_exception(pr), code(_code) {
      if (code!=0){
        fprintf(::stderr, "%s Exit process, code %d\n", pr->to_string().c_str(), _code);
        print_backtrace(pr->to_string());
      }
      msg = fmt::format("exit #{} <{}> code: {} at #{} <{}>", pr->pid(), pr->name(), _code, self()->pid(), self()->name());
    };
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
}
