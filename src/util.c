#include <string.h>

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
  int nr_of_solutions = 0;

  do {
    bool has_solution = a->compute_next_result(a, p);
    if(!has_solution) {
      return_code = 20;
      break;
    } else {
      ++solution;
      return_code = 10;

      if(cfg->print_options) {
        ++nr_of_solutions;
        for(xcc_link o = 0; o < p->l; ++o) {
          xcc_link o_ = p->x[o];

          // Go back to beginning of option
          while(TOP(o_ - 1) > 0)
            --o_;

          // This makes printing prettier. With algorithm M, options may be
          // empty, as branches are taken to resolve multiplicities. These are
          // expressed in the solution array, but don't directly correspond to
          // selected options.
          if(o_ > p->N && o_ <= p->Z) {
            while(TOP(o_) > 0) {
              printf("%s", NAME(TOP(o_)));
              if(TOP(o_) < p->color_size && COLOR(TOP(o_)) > 0) {
                printf(":%s", p->color_name[COLOR(TOP(o_))]);
              }
              ++o_;

              if(TOP(o_) > 0)
                printf(" ");
            }
            printf(";\n");
          }
        }
      } else if(cfg->print_x) {
        ++nr_of_solutions;
        for(size_t i = 0; i < p->l; ++i) {
          printf("%d ", p->x[i]);
        }
        printf("\n");
      } else {
        xcc_link solution[p->l];
        xcc_link l = xcc_extract_solution_option_indices(p, solution);
        if(l > 0) {
          for(size_t i = 0; i < l; ++i) {
            printf("%d ", solution[i]);
          }
          printf("\n");
          ++nr_of_solutions;
        }
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
    printf("Found %d solutions!\n", nr_of_solutions);
  }

  return return_code;
}

int
xcc_solve_problem(struct xcc_algorithm* a, struct xcc_problem* p) {
  assert(a);
  assert(p);
  bool has_result = a->compute_next_result(a, p);
  if(has_result)
    return 10;
  else
    return 20;
}

void
xcc_iterate_solution_options_str(struct xcc_problem* p,
                                 xcc_option_str_iterator it,
                                void* userdata) {
  assert(p);
  assert(it);
  const char* names[p->longest_option];
  const char* colors[p->longest_option];

  for(xcc_link o = 0; o < p->l; ++o) {
    memset(names, 0, p->longest_option);
    memset(colors, 0, p->longest_option);
    size_t i = 0;

    xcc_link o_ = p->x[o];

    // Go back to beginning of option
    while(TOP(o_ - 1) > 0)
      --o_;

    // This makes printing prettier. With algorithm M, options may be
    // empty, as branches are taken to resolve multiplicities. These are
    // expressed in the solution array, but don't directly correspond to
    // selected options.
    if(o_ > p->N && o_ <= p->Z) {
      while(TOP(o_) > 0) {
        names[i] = NAME(TOP(o_));
        if(TOP(o_) < p->color_size && COLOR(TOP(o_)) > 0) {
          colors[i] = p->color_name[COLOR(TOP(o_))];
        } else {
          colors[i] = NULL;
        }
        ++o_;
        ++i;
      }
      it(p, names, colors, i, userdata);
      i = 0;
    }
  }
}
