#include <string>
#include <vector>
#include <fmt/format.h>
#include "io/file.hpp"
#include "io/tcp/client.hpp"
#include "logger.hpp"

int main(int argc, char **argv){
  opp::start();

  if (argc!=3){
    OPP_ERROR("Requires 3 arguments");
    opp::stop();
    return 1;
  }

  auto client = opp::start<opp::io::tcp::client>(argv[1], argv[2]);
  // client->set_debug(true);
  // opp::self()->set_debug(true);

  opp::io::buffer_t data(1024);
  std::string str;
  unsigned int n=0;
  while(!client->eof()){
    client->write(fmt::format("This is a test {}\n", n));
    client->read(data);

    // str.clear();
    // std::copy(data.begin(), data.end(), std::back_inserter(str));
    // OPP_INFO(str);
    n+=1;
  }
  client->stop();

  OPP_INFO("EOF");
  opp::stop();
}
