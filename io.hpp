#include "process.hpp"

namespace popc::IO{
  class write_error : public std::exception{
    virtual const char* what() const throw(){
      return "write error";
    }
  };

  class File : popc::Process{
    std::string filename;
    int fd;
  public:
    File(std::string &&filename, int fd);
    File(std::string &&filename);
    ~File();
    void process();


    // out funcs
    void print(const std::string &);
    // in funcs
    std::string readline();
  };


  static File stdin("stdin", 0);
  static File stdout("stdout", 1);
  static File stderr("stderr", 2);
};
