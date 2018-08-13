#include <ostream>

namespace popc{
  class Symbol{
    int _id;
    const char *_name;
  public:
    Symbol();
    Symbol(const char *name);
    bool operator<(const Symbol &other) const {
      return this->_id < other._id;
    };
    bool operator==(const Symbol &other) const {
      return this->_id == other._id;
    };
    const char *name() const { return _name; }
    int id() const{ return _id; }
  };

  std::ostream& operator<< (std::ostream& os, const Symbol &sym);
}
