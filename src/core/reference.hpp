#pragma once

#include <ostream>

namespace opp{
  class reference{
    int64_t _id;
  public:
    reference();
    bool operator<(const reference &other) const {
      return this->_id < other._id;
    };
    bool operator==(const reference &other) const {
      return this->_id == other._id;
    };
    int id() const{ return _id; }

    std::string to_string();
  };

  inline reference make_reference(){
    return reference();
  }
}
