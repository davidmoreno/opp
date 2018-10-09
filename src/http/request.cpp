#include <fmt/printf.h>
#include "io/io.hpp"
#include "request.hpp"
#include "string.hpp"


opp::http::request::request(){
  path = "";
  method = "GET";
}

bool opp::http::request::parse(const opp::io::buffer_t &data){
  auto datav = std::string_view((const char*)data.data(), data.size());

  // fmt::print("-- {} --\n\n", data);

  auto lines = opp::string::split(datav, "\n\r");
  auto req_h = opp::string::split(lines[0]);
  method = req_h[0];
  path = req_h[1];

  headers.clear();

  auto I = std::begin(lines);
  ++I;
  auto endI = std::end(lines);
  std::string_view key, value;
  for(;I!=endI; ++I){
    // fmt::print("-- {} / {} -- \n", *I, I->size());
    if (*I == "")
      break;
    auto ls = opp::string::split(*I, ":");
    key = opp::string::strip(ls[0]);
    value = opp::string::strip(ls[1]);
    // fmt::print("-- {} -- {} -- \n", key, value);
    headers[key] = value;
  }
  // fmt::print("request ready\n");
  // fmt::print("{} {}\n", method, path);

  return true;
}
