#include <thread>

namespace popc{
  class Process;

  class VM{
    std::vector<std::thread> threads;
  public:
    void loop();
    void loop_thread();

    Process *self();
    void start_process(Process *pr);
  };
}
