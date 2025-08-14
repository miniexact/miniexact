#include <string.h>

#include <miniexact/algorithm.h>
#include <miniexact/log.h>
#include <miniexact/miniexact.h>
#include <miniexact/ops.h>
#include <miniexact/parse.h>
#include <miniexact/util.h>

int
miniexact_solve_problem_and_print_solutions(struct miniexact_algorithm* a,
                                            struct miniexact_problem* p,
                                            struct miniexact_config* cfg) {
  int return_code = EXIT_SUCCESS;
  if(!a->compute_next_result) {
    miniexact_err("Algorithm does not support solving!");
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
      // Fix for spurious solutions. This seems like an edge-case in the solver?
      if(p->option_count == 1 && solution > 0) {
        break;
      }
      ++solution;
      return_code = 10;

      if(cfg->print_options) {
        ++nr_of_solutions;
        for(miniexact_link o = 0; o < p->l; ++o) {
          miniexact_link o_ = p->x[o];

          // Go back to beginning of option
          while(TOP(o_ - 1) > 0)
            --o_;

          // This makes printing prettier. With algorithm M, options may be
          // empty, as branches are taken to resolve multiplicities. These are
          // expressed in the solution array, but don't directly correspond to
          // selected options.
          if(o_ > p->N && o_ <= p->Z) {
            while(TOP(o_) > 0) {
              if(NAME(TOP(o_)))
                printf("%s", NAME(TOP(o_)));
              else
                printf("%d", TOP(o_));
              if(o_ < p->color_size && COLOR(o_) > 0) {
                if(COLOR(o_) < p->color_name_size && p->color_name[COLOR(o_)])
                  printf(":%s", p->color_name[COLOR(o_)]);
                else
                  printf(":%d", COLOR(o_));
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
        miniexact_link *solution = malloc(sizeof(miniexact_link) * p->l);
        miniexact_link l =
          miniexact_extract_solution_option_indices(p, solution);
        if(l > 0) {
          for(size_t i = 0; i < l; ++i) {
            printf("%d ", solution[i]);
          }
          printf("\n");
          ++nr_of_solutions;
        }
        free(solution);
      }
    }
    if(cfg->enumerate)
      printf("\n");

    if(cfg->verbose) {
      miniexact_print_problem_matrix(p);
      printf("\n");
    }
  } while(cfg->enumerate);

  if(cfg->enumerate) {
    printf("Found %d solutions!\n", nr_of_solutions);
  }

  return return_code;
}

int
miniexact_solve_problem(struct miniexact_algorithm* a,
                        struct miniexact_problem* p) {
  assert(a);
  assert(p);
  bool has_result = a->compute_next_result(a, p);
  if(has_result)
    return 10;
  else
    return 20;
}

void
miniexact_iterate_solution_options_str(struct miniexact_problem* p,
                                       miniexact_option_str_iterator it,
                                       void* userdata) {
  assert(p);
  assert(it);
  const char** names = malloc(sizeof(char*) * p->longest_option);
  const char** colors = malloc(sizeof(char*) * p->longest_option);

  for(miniexact_link o = 0; o < p->l; ++o) {
    memset(names, 0, p->longest_option);
    memset(colors, 0, p->longest_option);
    size_t i = 0;

    miniexact_link o_ = p->x[o];

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

  free(colors);
  free(names);
}
