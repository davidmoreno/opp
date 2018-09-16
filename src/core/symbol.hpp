#include <ostream>

namespace opp{
  class symbol{
    int64_t _id;
    const char *_name;
  public:
    symbol();
    symbol(const char *name);
    bool operator<(const symbol &other) const {
      return this->_id < other._id;
    };
    bool operator==(const symbol &other) const {
      return this->_id == other._id;
    };
    const char *name() const { return _name; }
    int id() const{ return _id; }

    std::string to_string();
  };
}
