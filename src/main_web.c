#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include <xcc/algorithm.h>
#include <xcc/xcc.h>

__attribute__((export_name("solve"))) bool
solve(const char* problem,
      char algorithm,
      char enumerate,
      char verbose,
      void (*output)(const char*),
      void (*error)(const char*)) {
  int algorithm_select = 0;
  switch(algorithm) {
    case 'x':
      algorithm_select |= XCC_ALGORITHM_X;
      break;
    case 'c':
      algorithm_select |= XCC_ALGORITHM_C;
      break;
    case 'm':
      algorithm_select |= XCC_ALGORITHM_M;
      break;
    default:
      error("Unknown algorithm!");
      return false;
  }
  xcc_algorithm a;
  xcc_problem *p = xcc_problem_allocate();
  memset(&a, 0, sizeof(a));
  xcc_algorithm_from_select(algorithm_select, &a);
  xcc_problem_free(p, &a);
  return true;
}

int
main(int argc, char* argv[]) {
  return EXIT_SUCCESS;
}
