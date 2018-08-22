#include <sstream>

namespace opp::term{
  enum Color{
    RED=31,
    GREEN=32,
    YELLOW=33,
    BLUE=34,
    PURPLE=35,
    ORANGE=36,
    WHITE=37,
  };

  static std::string color(const std::string &str, Color color, bool highlight=false){
    std::ostringstream ss;
    int hl = highlight ? 1 : 0;
    ss<<"\033["<<hl<<';'<<color<<'m'<<str<<"\033[0m";
    return ss.str();
  }
  static std::string color(const std::string &str, Color color, Color bgcolor, bool highlight=false){
    std::ostringstream ss;
    int hl = highlight ? 1 : 0;
    ss<<"\033["<<hl<<';'<<color<<';'<<(bgcolor+10)<<'m'<<str<<"\033[0m";
    return ss.str();
  }
}
