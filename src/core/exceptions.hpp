#pragma once

#include <exception>
#include <memory>
#include <execinfo.h>
#include <cxxabi.h>
#include "process.hpp"
#include "string.hpp"
#include "opp.hpp"

namespace opp{
  class exception : public std::exception {
    void *trace[16];
    size_t trace_size;
  public:
    exception(){
      trace_size = backtrace(trace, 16);
    }
    void print_backtrace(std::string name="??"){
      opp::print_backtrace(std::move(name), trace, trace_size);
    }
  };

  class process_exception : public opp::exception {
  public:
    std::shared_ptr<opp::process> process;
    process_exception(std::shared_ptr<opp::process> pr) : process(pr){
      // print_backtrace();
    };
    const char *what() const noexcept{
      return "Receive on wrong process. Only currently executing process can call receive.";
    }
  };

  class bad_cast : public opp::exception {
  public:
    std::string msg;
    bad_cast(std::string a, std::string b){
      char *adm = abi::__cxa_demangle(a.c_str(), nullptr, nullptr, nullptr);
      char *bdm = abi::__cxa_demangle(b.c_str(), nullptr, nullptr, nullptr);
      msg = concat(
        "Bad cast. From ",
        adm ? adm : a,
        " to ",
        bdm ? bdm : b
      );
    }
    const char *what() const noexcept{
      return msg.c_str();
    }
  };

  class bad_receiver : public opp::process_exception {
  public:
    std::string msg;
    bad_receiver(std::shared_ptr<opp::process> pr) : process_exception(pr){
      // print_backtrace();
      msg = concat("bad receiver. Should be ", to_string(pr));
    };
    const char *what() const noexcept{
      return msg.c_str();
    }
  };

  class process_timeout : public opp::process_exception {
  public:
    std::string msg;
    process_timeout(std::shared_ptr<opp::process> pr) : process_exception(pr){
      msg = concat("timeout ", to_string(pr));
    };
    const char *what() const noexcept{
      return msg.c_str();
    }
  };

  class process_exit : public opp::process_exception {
  public:
    int code; // 0 controlled
    std::string description;
    process_exit(std::shared_ptr<opp::process> pr, int _code) : process_exception(pr), code(_code) {
      if (code!=0){
        print_backtrace();
      }
      description = concat("exit #", pr->pid(), "<", pr->name(), "> code: ", _code, " at #", self()->pid(), "<", self()->name(), ">");
    };
    const char *what() const noexcept{
      return description.c_str();
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

}
