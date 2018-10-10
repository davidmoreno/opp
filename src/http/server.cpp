#include "opp.hpp"
#include "task.hpp"

#include "string.hpp"
#include "server.hpp"
#include "logger.hpp"
#include "term.hpp"
#include "io/tcp/peer.hpp"


opp::http::server::server(std::string address, std::string port) :
    process("http server"),
    tcp_server(opp::start<opp::io::tcp::server>(address, port)),
    url("", [](const opp::http::request &){ return opp::http::response("not configured"); })
{

}

void opp::http::server::loop(){
  OPP_DEBUG("Serving...");

  while(running()){
    auto peer = tcp_server->wait_peer(opp::process::FOREVER);

    opp::task::start([this, peer]{ serve(peer); });
  }
}

void opp::http::server::serve(std::shared_ptr<opp::io::tcp::peer> peer){
  opp::io::buffer_t data(1024); // no request bigger than this
  self()->flags.silent_exception_exit = true;

  opp::http::request req;
  while(peer->running()){
    peer->read(data);
    req.parse(data);
    // fmt::print("Got request {}: {}\n", req.path, req.headers["User-Agent"]);

    auto f = url.find(req.path);
    if (f){
      auto response = f.value()(req);
      OPP_INFO("{} {} HTTP/1.1 {} {} - \"{}\"", req.method, req.path, response.status_code, response.content.size(), req.headers["User-Agent"]);

      response.write_to(peer);
    }
  }

  peer->close();
}
