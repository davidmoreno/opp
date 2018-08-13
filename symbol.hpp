
namespace popc{
  class Symbol{
    int id;
  public:
    Symbol();
    bool operator<(const Symbol &other) const {
      return this->id < other.id;
    };
  };
}
