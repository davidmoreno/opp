#include <iostream>
#include <unistd.h>
#include "file.hpp"
#include "core/opp.hpp"
#include "logger.hpp"
#include "poller.hpp"
#include "core/reference.hpp"
#include "core/exceptions.hpp"
#include "io/exceptions.hpp"

namespace opp::io{
  struct print_msg{ std::string str; };
  struct readline_msg{ std::shared_ptr<opp::process> from; };
  struct readline_result_msg{ std::string string; };
  struct read_msg{ reference ref; buffer_t &data; std::shared_ptr<opp::process> from; };
  struct write_msg{ reference ref; buffer_t &data; std::shared_ptr<opp::process> from; };
  struct close_msg{};
  struct read_result_msg{ reference ref; };
  struct write_result_msg{ reference ref; };

  std::shared_ptr<file> stdin;
  std::shared_ptr<file> stdout;
  std::shared_ptr<file> stderr;

  file::file(std::string name, int fd) : process(name), filename(std::move(name)), fd(fd){
    if (poller.use_count() == 0){ // Initialize once
      poller = std::make_shared<poller_t>();
    }
  }

  file::~file(){
    close();
  }

  /// Public API
  void file::replace_fd(int nfd){
    if (fd != -1){
      throw opp::io::exception("fd already initialized");
    }
    fd = nfd;
  }

  void file::print_(std::string str){
    send(print_msg{std::move(str)});
  }

  std::string file::readline(){
    send(readline_msg{opp::self()});
    auto res = opp::self()->receive({
      match_type<readline_result_msg>()
    }, process::FOREVER);
    return std::any_cast<readline_result_msg>(res).string;
  }

  void file::write(const std::string &str){
    auto data = buffer_t(str);
    write(data);
  }

  void file::write(buffer_t &data){
    auto ref = make_reference();
    send(write_msg{ref, data, self()});
    self()->receive({
      match_ref<write_result_msg>(ref)
    });
  }

  void file::read(buffer_t &data){
    auto ref = make_reference();
    send(read_msg{ref, data, self()});
    self()->receive({
      match_ref<read_result_msg>(ref)
    });
  }

  bool file::eof(){
    return !running();
  }

  void file::stop(){
    close();
    // fprintf(::stderr, "Force stop %s\n", name().c_str());
    this->process::stop();
  }

  void file::close(){
    send(close_msg{});
  }

  /// Server impl
  void file::loop(){
    // printf("process file %s %d\n", filename.c_str(), fd);
    // Build once, use man times
    auto printfn = [this](print_msg msg){
      auto str = msg.str;
      auto wrote = ::write(this->fd, str.c_str(), str.size());
      if ((wrote < 0) || (unsigned(wrote) < str.size())){
        throw write_error();
      }
    };
    auto readlinefn = [this](readline_msg msg){
      poller->wait_read(fd);
      std::string ret="";
      char c;
      do{
        ssize_t n = ::read(fd, &c, 1);
        if (n!=1){
          OPP_DEBUG("stdin closed");
          msg.from->send(exit_msg{shared_from_this()});
          stop();
          return;
        }
        ret+=c;
      }while(c!='\n');
      msg.from->send(readline_result_msg{ret});
    };
    auto write = [this](write_msg msg){
      poller->wait_write(fd);
      auto res = ::write(fd, msg.data.data(), msg.data.size());
      if (res<0){
        throw opp::io::write_error();
      }
      msg.from->send(write_result_msg{msg.ref});
    };
    auto read = [this](read_msg msg){
      poller->wait_read(fd);
      auto res = ::read(fd, msg.data.data(), msg.data.capacity());
      if (res<0){
        throw opp::io::write_error();
      }
      msg.data.set_size(res);
      msg.from->send(read_result_msg{msg.ref});
    };
    auto closef = [this](close_msg c){
      ::close(fd);
      fd = -1;
    };

    // std::map<symbol, std::function<void(const std::any &)>> _case = {
    //   {PRINT, [&printfn](const std::any &args){
    //     auto msg = std::any_cast<print_msg>(args);
    //     printfn(msg);;
    //   }},
    //   {READLINE, [&readlinefn](const std::any &args){
    //     auto msg = std::any_cast<readline_msg>(args);
    //     readlinefn(msg);
    //     // printf("%s: Answer for %s\n", name().c_str(), from->name().c_str());
    //   }}
    // };

    std::initializer_list<match_case> cases = {
      printfn, readlinefn, write, read, closef
    };


    while(running()){ // This will exit because of an exception when closed
      receive(cases, FOREVER);
    }
  }
}
