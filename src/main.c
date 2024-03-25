/*
    miniexact - Toolset to solve exact cover problems and extensions
    Copyright (C) 2021-2023  Maximilian Heisinger

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU Affero General Public License as
    published by the Free Software Foundation, either version 3 of the
    License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Affero General Public License for more details.

    You should have received a copy of the GNU Affero General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/
#include "miniexact/util.h"
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <miniexact/algorithm.h>
#include <miniexact/git.h>
#include <miniexact/log.h>
#include <miniexact/miniexact.h>
#include <miniexact/ops.h>
#include <miniexact/parse.h>

static void
print_help(void) {
  printf("miniexact -- solve XCC problems using different algorithms, version " MINIEXACT_VERSION "\n");
  printf("OPTIONS:\n");
  printf("  -h\t\tprint help\n");
  printf("  -p\t\tprint selected options\n");
  printf("  -e\t\tenumerate all solutions\n");
  printf("  -E\t\tprint the problem matrix in libExact format (only -x)\n");
  printf("ALGORITHM SELECTORS:\n");
  printf("  --naive\tuse naive in-order for i selection\n");
  printf("  --mrv\t\tuse MRV for i selection (default)\n");
  printf("  --smrv\tuse slack-aware MRV for i selection (default for M)\n    "
         "    \t    (see answer to ex. 166, p. 271)\n");
  printf("  -x\t\tuse Algorithm X\n");
  printf("  -c\t\tuse Algorithm C\n");
  printf("  -m\t\tuse Algorithm M\n");
  printf("  -k\t\tcall external binary to solve with SAT\n    \t\t    (Knuth's "
         "trivial encoding)\n");
}

static void
print_version(void) {
  printf("%s\n", MINIEXACT_VERSION);
}

static void
parse_cli(miniexact_config* cfg, int argc, char* argv[]) {
  int c;

  int sel[5];
  memset(sel, 0, sizeof(sel));

  struct option long_options[] = {
    { "verbose", no_argument, &cfg->verbose, 1 },
    { "help", no_argument, 0, 'h' },
    { "version", no_argument, 0, 'v' },
    { "print", no_argument, 0, 'p' },
    { "print-x", no_argument, 0, MINIEXACT_OPTION_PRINT_X },
    { "enumerate", no_argument, 0, 'e' },
    { "naive", no_argument, &sel[0], MINIEXACT_ALGORITHM_NAIVE },
    { "mrv", no_argument, &sel[1], MINIEXACT_ALGORITHM_MRV },
    { "smrv", no_argument, &sel[1], MINIEXACT_ALGORITHM_MRV },
    { "x", no_argument, &sel[2], MINIEXACT_ALGORITHM_X },
    { "c", no_argument, &sel[3], MINIEXACT_ALGORITHM_C },
    { "m", no_argument, &sel[3], MINIEXACT_ALGORITHM_M },
    { "k", no_argument, &sel[4], MINIEXACT_ALGORITHM_KNUTH_CNF },
    { 0, 0, 0, 0 }
  };

  while(1) {

    int option_index = 0;

    c = getopt_long(argc, argv, "eEpsxcmkhVv", long_options, &option_index);

    if(c == -1)
      break;

    switch(c) {
      case 'V':
        cfg->verbose = 1;
        break;
      case 'v':
        print_version();
        exit(EXIT_SUCCESS);
      case 'p':
        cfg->print_options = 1;
        break;
      case MINIEXACT_OPTION_PRINT_X:
        cfg->print_x = 1;
        break;
      case 'e':
        cfg->enumerate = 1;
        break;
      case 'E':
        cfg->transform_to_libexact = 1;
        break;
      case 'h':
        print_help();
        exit(EXIT_SUCCESS);
      case 'x':
        cfg->algorithm_select |= MINIEXACT_ALGORITHM_X;
        break;
      case 'c':
        cfg->algorithm_select |= MINIEXACT_ALGORITHM_C;
        break;
      case 'm':
        cfg->algorithm_select |= MINIEXACT_ALGORITHM_M;
        break;
      case 'k':
        cfg->algorithm_select |= MINIEXACT_ALGORITHM_KNUTH_CNF;
        break;
      default:
        break;
    }
  }

  if(optind < argc) {
    cfg->input_files = &argv[optind];
    cfg->input_files_count = argc - optind;
  }

  for(size_t i = 0; i < sizeof(sel) / sizeof(sel[0]); ++i)
    cfg->algorithm_select |= sel[i];
}

static int
process_file(miniexact_config* cfg) {
  miniexact_algorithm a;
  if(!miniexact_algorithm_from_select(cfg->algorithm_select, &a)) {
    miniexact_err(
      "Could not extract algorithm from algorithm select! Try different "
      "algorithm selection.");
    return EXIT_FAILURE;
  }

  miniexact_problem* p =
    miniexact_parse_problem_file(&a, cfg->input_files[cfg->current_input_file]);
  if(!p)
    return EXIT_FAILURE;

  p->cfg = cfg;

  if(cfg->verbose)
    miniexact_print_problem_matrix(p);

  if(cfg->transform_to_libexact) {
    const char* error = miniexact_print_problem_matrix_in_libexact_format(p);
    if(error) {
      miniexact_err("Transform error: %s", error);
      return EXIT_FAILURE;
    } else
      return EXIT_SUCCESS;
  }

  int return_code = miniexact_solve_problem_and_print_solutions(&a, p, cfg);

  miniexact_problem_free(p, &a);
  return return_code;
}

int
main(int argc, char* argv[]) {
  miniexact_config cfg;
  memset(&cfg, 0, sizeof(cfg));
  parse_cli(&cfg, argc, argv);

  int status = EXIT_FAILURE;

  if(cfg.input_files) {
    for(cfg.current_input_file = 0;
        cfg.current_input_file < cfg.input_files_count;
        ++cfg.current_input_file) {
      if(cfg.input_files_count > 1) {
        if(cfg.current_input_file > 0)
          printf("\n");
        printf(">>> %s <<<\n", cfg.input_files[cfg.current_input_file]);
      }
      status = process_file(&cfg);
    }
  }

  return status;
}
