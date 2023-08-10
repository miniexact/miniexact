#include <xcc/algorithm.h>
#include <xcc/log.h>
#include <xcc/ops.h>
#include <xcc/parse.h>
#include <xcc/util.h>
#include <xcc/xcc.h>

int
xcc_solve_problem_and_print_solutions(struct xcc_algorithm* a,
                                      struct xcc_problem* p,
                                      struct xcc_config* cfg) {
  int return_code = EXIT_SUCCESS;
  if(!a->compute_next_result) {
    err("Algorithm does not support solving!");
    return EXIT_FAILURE;
  }

  int solution = 0;

  do {
    bool has_solution = a->compute_next_result(a, p);
    if(!has_solution) {
      return_code = 20;
      break;
    } else {
      ++solution;
      return_code = 10;

      if(cfg->print_options) {
        for(xcc_link o = 0; o < p->l; ++o) {
          xcc_link o_ = p->x[o];

          // Go back to beginning of option
          while(TOP(o_ - 1) > 0)
            --o_;

          while(TOP(o_) > 0) {
            printf("%s", NAME(TOP(o_)));
            if(o_ < p->color_size && COLOR(o_) != 0)
              printf(":%s", p->color_name[o_]);
            ++o_;

            if(TOP(o_) > 0)
              printf(" ");
          }
          printf(";\n");
        }
      } else {
        xcc_link solution[p->l];
        xcc_extract_solution_option_indices(p, solution);
        for(size_t i = 0; i < p->l; ++i) {
          printf("%d ", solution[i]);
        }
        printf("\n");
      }
    }
    if(cfg->enumerate)
      printf("\n");

    if(cfg->verbose) {
      xcc_print_problem_matrix(p);
      printf("\n");
    }
  } while(cfg->enumerate);

  if(cfg->enumerate) {
    printf("Found %d solutions!\n", solution);
  }

  return return_code;
}
