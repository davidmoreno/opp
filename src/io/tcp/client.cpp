#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>


#include "client.hpp"
#include "../../logger.hpp"

using namespace opp;

io::tcp::client::client(std::string address, std::string port) : io::file(address+":"+port, -1){
  int sockfd;
  if (debug()){
    OPP_DEBUG("connect to {}:{}", address, port);
  }

  struct addrinfo hints, *servers;
  memset (&hints, 0, sizeof (hints));
  hints.ai_family = PF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags |= AI_CANONNAME;

  auto error = getaddrinfo(
    address.c_str(), port.c_str(),
    &hints, &servers
  );

  if (error != 0){
    throw opp::io::exception(gai_strerror(error));
  }

  char addrstr[100];
  struct addrinfo *serverI = servers;
  bool connected = false;

  while (serverI) {
    if (debug()){
      void *ptr;
      inet_ntop (serverI->ai_family, serverI->ai_addr->sa_data, addrstr, sizeof(addrstr));

      switch (serverI->ai_family)
        {
        case AF_INET:
          ptr = &((struct sockaddr_in *) serverI->ai_addr)->sin_addr;
          break;
        case AF_INET6:
          ptr = &((struct sockaddr_in6 *) serverI->ai_addr)->sin6_addr;
          break;
        }
      inet_ntop (serverI->ai_family, ptr, addrstr, sizeof(addrstr));

      OPP_DEBUG(
        "Try connect IPv{} address: {} ({}:{})",
        serverI->ai_family == PF_INET6 ? 6 : 4,
        addrstr, serverI->ai_canonname,
        port
      );
    }

    if ((sockfd = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
      throw opp::io::exception("cant-create-socket");
    }
    if (connect(sockfd, serverI->ai_addr, serverI->ai_addrlen) >= 0){
      if (debug()){
        OPP_DEBUG("Connected!");
      }
      connected = true;
      break;
    }
    ::close(sockfd);

    serverI = serverI->ai_next;
  }
  freeaddrinfo(servers);
  if (connected){
    replace_fd(sockfd);
  } else {
    throw opp::io::exception(strerror(errno));
  }
}
