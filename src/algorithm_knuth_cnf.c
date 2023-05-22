#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <xcc/algorithm.h>
#include <xcc/algorithm_knuth_cnf.h>
#include <xcc/log.h>
#include <xcc/ops.h>
#include <xcc/sat_solver.h>

struct algorithm_knuth_cnf {
  xcc_sat_solver solver;
  xcc_link* past_solutions;
  size_t past_solutions_size;
  size_t past_solutions_count;
};

static struct algorithm_knuth_cnf*
create_k() {
  struct algorithm_knuth_cnf* k = calloc(1, sizeof(struct algorithm_knuth_cnf));
  k->past_solutions = NULL;
  k->past_solutions_size = 0;
  k->past_solutions_count = 0;
  return k;
}

static inline xcc_link
get_option_id(xcc_problem* p, xcc_link i) {
  while(TOP(i) > 0)
    ++i;
  return -TOP(i);
}

struct vars_clauses {
  int variables;
  int clauses;
};
static struct vars_clauses
compute_vars_clauses(xcc_problem* p) {
  struct algorithm_knuth_cnf* k = p->algorithm_userdata;
  xcc_sat_solver* s = &k->solver;

  struct vars_clauses v = { p->option_count, 0 };

  // Go downwards from items so that every option is captured.
  for(xcc_link i = 1; i <= p->N_1; ++i) {
    ++v.clauses;
    for(xcc_link down1 = DLINK(i); down1 > ULINK(down1); down1 = DLINK(down1)) {
      xcc_link option1 = get_option_id(p, down1);
      for(xcc_link down2 = DLINK(i); down2 > ULINK(down2);
          down2 = DLINK(down2)) {
        if(down1 == down2)
          continue;
        ++v.clauses;
      }
    }
  }

  return v;
}

static void
encode_problem(xcc_problem* p, size_t additional_clauses) {
  struct algorithm_knuth_cnf* k = p->algorithm_userdata;
  xcc_sat_solver* s = &k->solver;

  struct vars_clauses v = compute_vars_clauses(p);
  xcc_sat_solver_find_and_init(s, v.variables, v.clauses + additional_clauses);

  if(p->N != p->N_1) {
    err("No secondary items supported by the SAT solver (yet)!");
    return;
  }

  // Go downwards from items so that every option is captured.
  for(xcc_link i = 1; i <= p->N_1; ++i) {
    for(xcc_link down = DLINK(i); down > ULINK(down); down = DLINK(down)) {
      xcc_link option = get_option_id(p, down);
      xcc_sat_solver_add(s, option);
    }
    xcc_sat_solver_add(s, 0);
    for(xcc_link down1 = DLINK(i); down1 > ULINK(down1); down1 = DLINK(down1)) {
      xcc_link option1 = get_option_id(p, down1);
      for(xcc_link down2 = DLINK(i); down2 > ULINK(down2);
          down2 = DLINK(down2)) {
        if(down1 == down2)
          continue;
        xcc_link option2 = get_option_id(p, down2);
        xcc_sat_solver_binary(s, -option1, -option2);
      }
    }
  }
}

static bool
compute_next_result(xcc_algorithm* a, xcc_problem* p) {
  if(!p->algorithm_userdata) {
    p->algorithm_userdata = create_k();
  }
  struct algorithm_knuth_cnf* k = p->algorithm_userdata;

  encode_problem(p, k->past_solutions_count + (p->x_size != 0 ? 1 : 0));

  if(p->x_size != 0) {
    // The last found result has to be added to the last solutions! This cost is
    // only paid if multiple solutions should be enumerated. Would be nicer with
    // incremental SAT, but without dependencies, this is what it is.
    //
    // +1 so that the trailing 0 is also saved.
    size_t new_size = k->past_solutions_size + p->x_size + 1;
    k->past_solutions = realloc(k->past_solutions, new_size * sizeof(int32_t));

    xcc_link options[p->x_size];
    xcc_extract_solution_option_indices(p, options);
    xcc_sat_solver* s = &k->solver;
    for(size_t i = 0; i < p->x_size; ++i) {
      k->past_solutions[k->past_solutions_size + i] = -options[i];
    }
    k->past_solutions[k->past_solutions_size + p->x_size] = 0;

    k->past_solutions_size = new_size;

    ++k->past_solutions_count;
  }

  for(size_t i = 0; i < k->past_solutions_size; ++i) {
    xcc_sat_solver_add(&k->solver, k->past_solutions[i]);
  }

  int r = xcc_sat_solver_solve(&k->solver);

  if(r == 20)
    return false;
  else if(r == 10) {
    p->x_size = 0;
    for(xcc_link i = p->N + 2; i <= p->Z; ++i) {
      xcc_link t = TOP(i);
      if(t < 0) {
        bool active = k->solver.assignments[-t];
        if(active) {
          XCC_ARR_PLUS1(x)
          p->x[p->x_size - 1] = i - 1;
        }
      }
    }
    p->l = p->x_size;
    return true;
  }

  return false;
}

static void
free_userdata(xcc_algorithm* a, xcc_problem* p) {
  if(!p->algorithm_userdata)
    return;

  struct algorithm_knuth_cnf* k = p->algorithm_userdata;
  if(k->solver.assignments) {
    free(k->solver.assignments);
    k->solver.assignments = NULL;
  }

  free(k);
}

void
xcc_algoritihm_knuth_cnf_set(xcc_algorithm* a) {
  xcc_algorithm_standard_functions(a);

  a->compute_next_result = &compute_next_result;
  a->free_userdata = &free_userdata;
}
