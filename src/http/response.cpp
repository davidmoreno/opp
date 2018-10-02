#include "response.hpp"

void opp::http::response::write_to(std::shared_ptr<opp::io::file> f){
  std::string all_response; // all in one str is more efficient

  all_response = fmt::format(
    "HTTP/1.1 {} OK\r\nContent-Type: {}\r\nContent-Length: {}\r\n\r\n{}",
    status_code, "text/html", content.size(), content
  );
  f->write(all_response);
}
