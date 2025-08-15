#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <string>

#include <emscripten.h>
#include <emscripten/bind.h>

#include <miniexact/algorithm.h>
#include <miniexact/log.h>
#include <miniexact/parse.h>
#include <miniexact/util.h>
#include <miniexact/miniexact.h>

// Giving arguments as std::string makes emscripten bindings easier.

extern "C" int
solve(std::string problem,
      std::string algorithm,
      std::string enumerate,
      std::string verbose,
      std::string print_options) {
  int algorithm_select = 0;
  if(algorithm == "x") {
    algorithm_select |= MINIEXACT_ALGORITHM_X;
  } else if(algorithm == "c") {
    algorithm_select |= MINIEXACT_ALGORITHM_C;
  } else if(algorithm == "m") {
    algorithm_select |= MINIEXACT_ALGORITHM_M;
  } else {
    miniexact_err("Unknown algorithm: %s", algorithm.c_str());
    return false;
  }
  miniexact_config cfg;
  memset(&cfg, 0, sizeof(cfg));
  cfg.algorithm_select = algorithm_select;
  cfg.enumerate = enumerate == "e";
  cfg.verbose = verbose == "v";
  cfg.print_options = print_options == "p";

  miniexact_algorithm a;
  memset(&a, 0, sizeof(a));
  miniexact_algorithm_from_select(algorithm_select, &a);
  miniexact_problem* p = miniexact_parse_problem(&a, problem.c_str(), NULL);

  if(!p)
    return EXIT_FAILURE;

  if(cfg.verbose)
    miniexact_print_problem_matrix(p);

  int status = miniexact_solve_problem_and_print_solutions(&a, p, &cfg);

  miniexact_problem_free(p, &a);
  return status;
}

std::string version() {
  return MINIEXACT_VERSION;
}

EMSCRIPTEN_BINDINGS(Module) {
  emscripten::function("solve", &solve);
  emscripten::function("version", &version);
}

int
main(int argc, char* argv[]) {
  return EXIT_SUCCESS;
}
