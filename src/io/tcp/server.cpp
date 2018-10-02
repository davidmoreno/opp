#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "opp.hpp"
#include "poller.hpp"
#include "process.hpp"
#include "logger.hpp"
#include "server.hpp"
#include "peer.hpp"

struct wait_peer_msg{ opp::reference ref; std::shared_ptr<opp::process> from; };
struct peer_msg{ opp::reference ref; std::shared_ptr<opp::io::tcp::peer> peer; };

opp::io::tcp::server::server(std::string address, std::string port) : opp::io::file(address+":"+port, -1){
  auto socket_fd = socket(PF_INET, SOCK_STREAM, 0);

  {
    int on = 1;
    if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(int)) < 0)
      throw opp::io::exception("setsockopt(SO_REUSEADDR) failed");
  }

	struct sockaddr_in sa;

  if ( socket_fd < 0 ){
    throw opp::io::exception("cant-create-socket");
  }

  memset(&sa, 0, sizeof(struct sockaddr_in));
  sa.sin_family = AF_INET;
  sa.sin_addr.s_addr = inet_addr(address.c_str());
  sa.sin_port = htons(atoi(port.c_str()));

  if (bind(socket_fd, (struct sockaddr *)&sa, sizeof(sa)) == -1){
    close(socket_fd);
    throw opp::io::exception(fmt::format("bind port failed {}:{}", address, port));
  }
  replace_fd(socket_fd);
  // receive listens at main_loop
}

void opp::io::tcp::server::loop(){
  struct sockaddr_in isa;
  int socket_fd = get_fd();
  // OPP_DEBUG("Waiting connections at {}, fd: {}", name(), get_fd());
  listen(socket_fd, 5);
  while(running()){
    receive({
      [&isa, socket_fd](wait_peer_msg msg){
        socklen_t addr_size = sizeof(isa);
        poller->wait_read(socket_fd);
        auto peer_fd = accept(socket_fd, (struct sockaddr*)&isa, &addr_size);
        if (peer_fd<0){
          OPP_ERROR("Error accepting connection.");
        }
        auto peer = opp::start<opp::io::tcp::peer>(peer_fd);
        // OPP_DEBUG("Send peer {} to: {}", std::to_string(peer), std::to_string(msg.from));
        msg.from->send(peer_msg{ msg.ref, peer });
      }
    }, FOREVER);
  }
}

std::shared_ptr<opp::io::tcp::peer> opp::io::tcp::server::wait_peer(){
  auto ref = opp::make_reference();
  this->send(wait_peer_msg{ref, self()});
  auto msg = self()->receive({
    match_ref<peer_msg>(ref)
  });
  return std::any_cast<peer_msg>(msg).peer;
}
