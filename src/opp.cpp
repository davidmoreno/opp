#include <execinfo.h>
#include <thread>
#include "process.hpp"
#include "vm.hpp"
#include "exceptions.hpp"

namespace opp{
  std::shared_ptr<process> self(){
    return vm->self();
  }

  void start(){
    new VM();
  }
  void stop(){
    if (!vm)
      throw opp::not_initialized();
    delete vm;
  }

  // from https://stackoverflow.com/questions/3151779/best-way-to-invoke-gdb-from-inside-program-to-print-its-stacktrace/4611112#4611112
  void print_backtrace(){
    void *trace[16];
    char **messages = (char **)NULL;
    int i, trace_size = 0;

    trace_size = backtrace(trace, 16);
    messages = backtrace_symbols(trace, trace_size);

    printf("[bt] Execution path:\n");
    for (i=1; i<trace_size; ++i)
    {
      fprintf(stderr, "[bt] #%d %s -- ", i, messages[i]);

      /* find first occurence of '(' or ' ' in message[i] and assume
       * everything before that is the file name. (Don't go beyond 0 though
       * (string terminator)*/
      size_t p = 0;
      while(messages[i][p] != '(' && messages[i][p] != ' '
              && messages[i][p] != 0)
          ++p;

      // Now I have the offset
      auto offset = strtol(&messages[i][p+1], NULL, 16);

      char syscom[256];
      sprintf(syscom,"addr2line %x -e %.*s", (int)offset, (int)p, messages[i]);
          //last parameter is the file name of the symbol
      system(syscom);
    }
  }
}
