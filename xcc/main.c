#include "parse.h"
#include "xcc.h"

#ifdef __linux__

#include <execinfo.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
void
handler(int sig) {
  void* array[10];
  size_t size;

  // get void*'s for all entries on the stack
  size = backtrace(array, 10);

  // print out all the frames to stderr
  fprintf(stderr, "Error: signal %d:\n", sig);
  backtrace_symbols_fd(array, size, STDERR_FILENO);
  exit(1);
}

#endif

int
main() {
#ifdef __linux__
  signal(SIGSEGV, handler);
#endif
  return xcc_parse_problem();
}
