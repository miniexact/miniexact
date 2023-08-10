#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <emscripten.h>
#include <emscripten/bind.h>

#include <xcc/algorithm.h>
#include <xcc/log.h>
#include <xcc/parse.h>
#include <xcc/util.h>
#include <xcc/xcc.h>

// Giving arguments as std::string makes emscripten bindings easier.

extern "C" int
solve(std::string problem,
      std::string algorithm,
      std::string enumerate,
      std::string verbose,
      std::string print_options) {
  int algorithm_select = 0;
  if(algorithm == "x") {
    algorithm_select |= XCC_ALGORITHM_X;
  } else if(algorithm == "c") {
    algorithm_select |= XCC_ALGORITHM_C;
  } else if(algorithm == "m") {
    algorithm_select |= XCC_ALGORITHM_M;
  } else {
    err("Unknown algorithm: %s", algorithm.c_str());
    return false;
  }
  xcc_config cfg;
  memset(&cfg, 0, sizeof(cfg));
  cfg.algorithm_select = algorithm_select;
  cfg.enumerate = enumerate == "e";
  cfg.verbose = verbose == "v";
  cfg.print_options = print_options == "p";

  xcc_algorithm a;
  memset(&a, 0, sizeof(a));
  xcc_algorithm_from_select(algorithm_select, &a);
  xcc_problem* p = xcc_parse_problem(&a, problem.c_str());

  if(!p)
    return EXIT_FAILURE;

  if(cfg.verbose)
    xcc_print_problem_matrix(p);

  int status = xcc_solve_problem_and_print_solutions(&a, p, &cfg);

  xcc_problem_free(p, &a);
  return status;
}

EMSCRIPTEN_BINDINGS(Module) {
  emscripten::function("solve", &solve);
}

int
main(int argc, char* argv[]) {
  return EXIT_SUCCESS;
}
