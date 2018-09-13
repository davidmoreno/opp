#include <string>
#include <vector>
#include "tcp.hpp"
#include "logger.hpp"

int main(int argc, char **argv){
  opp::start();

  if (argc!=3){
    OPP_ERROR("Requires 3 arguments");
    opp::stop();
    return 1;
  }

  auto client = opp::start<opp::tcp_client>(argv[1], argv[2]);
  client->set_debug(true);

  std::vector<int8_t> data(1024);
  std::string str;
  while(!client->eof()){
    client->write("This is a test\n");
    client->read(data);

    str.clear();
    std::copy(data.begin(), data.end(), std::back_inserter(str));
    OPP_INFO(str);
  }
  client->stop();

  OPP_INFO("EOF");
  opp::stop();
}
