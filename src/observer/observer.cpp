#include "observer.hpp"
#include "http/server.hpp"
#include "http/url.hpp"
#include "timer.hpp"

namespace opp{
  observer::observer(std::string &&uipath, std::string &&address, std::string &&port){
    http_server = opp::start<http::server>(address, port);

    auto htmlfile = opp::start<io::file>(uipath, io::read);

    html = std::move(htmlfile->read());
    OPP_DEBUG("Read Observer UI: {}, <{}>", std::to_string(html.size()), std::string_view((const char*)html.data(), html.size()) );

    http_server->set_root_url({
      {"/", [this](const http::request &req){ return this->index(req); }},
      {"/api/", [this](const http::request &req){ return this->api(req); }},
    });
  }
  void observer::loop(){
    while(running()){
      timer::sleep(std::chrono::seconds(1000));
    }
  }

  http::response observer::index(const http::request &req){
    return http::response(html);
  }

  http::response observer::api(const http::request &req){
    return http::response("{}");
  }

}
