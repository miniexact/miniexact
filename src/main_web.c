#include <stdbool.h>
#include <stdlib.h>

#include <xcc/xcc.h>

__attribute__((export_name("solve"))) bool
solve(const char* problem, char algorithm, char enumerate, char verbose) {
  return true;
}

int
main(int argc, char* argv[]) {
  return EXIT_SUCCESS;
}
