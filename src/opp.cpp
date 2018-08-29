#include <execinfo.h>
#include <thread>
#include <cxxabi.h>
#include "process.hpp"
#include "vm.hpp"
#include "exceptions.hpp"

namespace opp{
  std::shared_ptr<process> self(){
    return vm->self();
  }

  void start(){
    vm = std::make_shared<VM>();
    vm->start();
  }
  void stop(){
    if (!vm)
      throw opp::not_initialized();
    vm->stop();
    vm.reset();
  }

  // from https://stackoverflow.com/questions/3151779/best-way-to-invoke-gdb-from-inside-program-to-print-its-stacktrace/4611112#4611112
  void print_backtrace(std::string name){
    void *trace[16];
    char **messages = (char **)NULL;
    char temp[256];
    char temp2[256];
    int i, trace_size = 0;
    const char *cname = name.c_str();

    trace_size = backtrace(trace, 16);
    messages = backtrace_symbols(trace, trace_size);

    printf("[%s] Execution path:\n", cname);
    for (i=1; i<trace_size; ++i)
    {
      fprintf(stderr, "[%s] #%d %s -- ", cname, i, messages[i]);

      /* find first occurence of '(' or ' ' in message[i] and assume
       * everything before that is the file name. (Don't go beyond 0 though
       * (string terminator)*/
      size_t p = 0;
      while(messages[i][p] != '(' && messages[i][p] != ' '
              && messages[i][p] != 0)
          ++p;

      char *addr = &messages[i][p+1];
      if (*addr == '+'){
        // Now I have the offset
        auto offset = strtol(addr+1, NULL, 16);

        sprintf(temp,"addr2line 0x%X -e %.*s", (int)offset, (int)p, messages[i]);
            //last parameter is the file name of the symbol
        system(temp);
      } else {
        char *m = addr;
        char *r = temp;
        while(*m!='+' && *m!='\0'){
          *r = *m;
          ++r;
          ++m;
        }
        *r = '\0';

        size_t siz = sizeof(temp2);
        int st;
        fprintf(stderr, "%s\n", abi::__cxa_demangle(temp, temp2, &siz, &st));
      }
    }
    free(messages);
  }
}
