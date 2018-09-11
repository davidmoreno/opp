#include <ostream>

namespace opp{
  class symbol{
    int _id;
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
  };

  std::ostream& operator<< (std::ostream& os, const symbol &sym);
}
