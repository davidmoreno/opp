#include "opp.hpp"
#include "task.hpp"

#include "string.hpp"
#include "server.hpp"
#include "logger.hpp"
#include "io/tcp/peer.hpp"


opp::http::server::server(std::string address, std::string port) :
    tcp_server(opp::start<opp::io::tcp::server>(address, port)),
    url("", [](const opp::http::request &){ return opp::http::response("not configured"); }),
    process("http server")
{

}

void opp::http::server::loop(){
  OPP_DEBUG("Serving...");

  while(running()){
    auto peer = tcp_server->wait_peer();

    opp::task::start([this, peer]{ serve(peer); });
  }
}

void opp::http::server::serve(std::shared_ptr<opp::io::tcp::peer> peer){
  opp::io::file::buffer_t data(1024); // no request bigger than this

  peer->read(data);
  auto reqstr = std::to_string(data);
  auto lines = opp::string::split(reqstr, "\n\r");
  auto req_h = opp::string::split(lines[0]);
  opp::http::request req{std::string(req_h[0]), std::string(req_h[1])};

  auto f = url.find(req.path);
  if (f){
    auto response = f.value()(req);

    response.write_to(peer);
  }

  OPP_DEBUG("{}", std::to_string(lines));

  peer->close();
}
