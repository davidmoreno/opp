#pragma once

#include <ostream>

namespace opp{
  class reference{
    int64_t _id;
  public:
    reference(const reference &r){ _id = r._id; };
    reference();
    bool operator<(const reference &other) const {
      return this->_id < other._id;
    };
    bool operator==(const reference &other) const {
      return this->_id == other._id;
    };
    bool operator!=(const reference &other) const {
      return this->_id != other._id;
    };
    int id() const{ return _id; }

    std::string to_string() const;
  };

  inline reference make_reference(){
    return reference();
  }
}

namespace std{
  inline std::string to_string(const opp::reference &r){
    return r.to_string();
  }
};
