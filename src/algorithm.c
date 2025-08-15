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
#include "miniexact/miniexact.h"
#include <miniexact/algorithm.h>
#include <miniexact/algorithm_c.h>
#include <miniexact/algorithm_c_dollar.h>
#include <miniexact/algorithm_knuth_cnf.h>
#include <miniexact/algorithm_m.h>
#include <miniexact/algorithm_x.h>
#include <miniexact/ops.h>
#include <stdint.h>

static inline const char*
define_item(miniexact_algorithm* a, miniexact_problem* p, miniexact_link l) {
  assert(a);
  assert(p);
  assert(l);
  assert(p->llink);
  assert(p->rlink);

  MINIEXACT_ARR_PLUS1(llink)
  MINIEXACT_ARR_PLUS1(rlink)

  p->i = p->i + 1;
  LLINK(p->i) = p->i - 1;
  RLINK(p->i - 1) = p->i;

  return NULL;
}

static const char*
define_primary_item(miniexact_algorithm* a,
                    miniexact_problem* p,
                    miniexact_link l) {
  const char* e;
  if((e = define_item(a, p, l)))
    return e;

  ++p->primary_item_count;

  // Always track them with as the base-case w.r.t. primary items.
  MINIEXACT_ARR_PLUS1(slack)
  MINIEXACT_ARR_PLUS1(bound)
  SLACK(p->i) = 0;
  BOUND(p->i) = 1;

  return NULL;
}

// Adds a primary item with a range [u;v] (sets SLACK and BOUND)
static const char*
define_primary_item_with_range(miniexact_algorithm* a,
                               miniexact_problem* p,
                               miniexact_link l,
                               miniexact_link u,
                               miniexact_link v) {
  const char* e;
  if((e = define_item(a, p, l)))
    return e;

  if(u > v) {
    return "u must be smaller than v in the multiplicity range!";
  }
  if(v == 0) {
    return "v must not be 0 in the multiplicity range!";
  }

  ++p->primary_item_count;

  MINIEXACT_ARR_PLUS1(slack)
  MINIEXACT_ARR_PLUS1(bound)

  SLACK(p->i) = v - u;
  BOUND(p->i) = v;

  return NULL;
}

static const char*
define_secondary_item(miniexact_algorithm* a,
                      miniexact_problem* p,
                      miniexact_link l) {
  const char* e;

  if((e = define_item(a, p, l)))
    return e;

  if(p->secondary_item_count == 0) {
    p->N_1 = p->i - 1;
  }

  ++p->secondary_item_count;

  return NULL;
}

static const char*
prepare_options(miniexact_algorithm* a, miniexact_problem* p) {
  // Step I2
  p->N = p->i;
  if(p->N_1 < 0)
    p->N_1 = p->N;
  LLINK(p->N + 1) = p->N;
  RLINK(p->N) = p->N + 1;
  LLINK(p->N_1 + 1) = p->N + 1;
  RLINK(p->N + 1) = p->N_1 + 1;
  LLINK(0) = p->N_1;
  RLINK(p->N_1) = 0;

  // Step N3
  MINIEXACT_ARR_PLUSN(len, p->N + 2);
  MINIEXACT_ARR_PLUSN(ulink, p->N + 2);
  MINIEXACT_ARR_PLUSN(dlink, p->N + 2);
  MINIEXACT_ARR_PLUSN(color, p->N + 2);

  // Normalize the don't cares
  ULINK(p->N + 1) = 0;
  DLINK(p->N + 1) = 0;

  LEN(0) = 0;
  ULINK(0) = 0;
  DLINK(0) = 0;
  COLOR(0) = 0;

  for(int i = 1; i <= p->N; ++i) {
    LEN(i) = 0;
    ULINK(i) = i;
    DLINK(i) = i;
    COLOR(i) = 0;
  }

  p->M = 0;
  p->p = p->N + 1;
  TOP(p->p) = 0;
  COLOR(p->p) = 0;

  p->Z = p->p;
  return NULL;
}

static const char*
add_item_with_color(miniexact_algorithm* a,
                    miniexact_problem* p,
                    miniexact_link ij,
                    miniexact_color c) {
  if(ij < 1)
    return "Invalid ij given for add_item!";

  MINIEXACT_ARR_PLUS1(len)
  MINIEXACT_ARR_PLUS1(dlink)
  MINIEXACT_ARR_PLUS1(ulink)
  MINIEXACT_ARR_PLUS1(color)
  MINIEXACT_ARR_PLUS1(cost)

  ++p->j;

  LEN(ij) = LEN(ij) + 1;
  p->q = ULINK(ij);
  ULINK(p->p + p->j) = p->q;
  DLINK(p->q) = p->p + p->j;
  DLINK(p->p + p->j) = ij;
  ULINK(ij) = p->p + p->j;
  TOP(p->p + p->j) = ij;
  COLOR(p->p + p->j) = c;

  return NULL;
}

static const char*
add_item(miniexact_algorithm* a, miniexact_problem* p, miniexact_link ij) {
  return add_item_with_color(a, p, ij, 0);
}

static const char*
end_option(miniexact_algorithm* a, miniexact_problem* p, int32_t cost) {
  MINIEXACT_ARR_PLUS1(len)
  MINIEXACT_ARR_PLUS1(dlink)
  MINIEXACT_ARR_PLUS1(ulink)
  MINIEXACT_ARR_PLUS1(color)
  MINIEXACT_ARR_PLUS1(cost)

  if(cost < p->max_option_cost) {
    return "Costs are not ordered! Each option may have equal or increasing "
           "costs.";
  }
  p->max_option_cost = cost;
  for(miniexact_link i = p->p; i <= p->p + p->j + 1; ++i) {
    COST(i) = cost;
  }

  p->M = p->M + 1;
  DLINK(p->p) = p->p + p->j;
  p->p = p->p + p->j + 1;
  TOP(p->p) = -p->M;
  ULINK(p->p) = p->p - p->j;
  COLOR(p->p) = 0;

  p->longest_option = MAX(p->longest_option, p->j);

  p->j = 0;
  p->Z = p->p;

  return NULL;
}

static const char*
end_options(miniexact_algorithm* a, miniexact_problem* p) {
  DLINK(p->dlink_size - 1) = 0;
  return NULL;
}

const char*
miniexact_default_init_problem(miniexact_algorithm* a, miniexact_problem* p) {
  assert(a);
  assert(p);

  p->algorithm_userdata = NULL;

  MINIEXACT_ARR_ALLOC(miniexact_link, llink)
  MINIEXACT_ARR_ALLOC(miniexact_link, rlink)
  MINIEXACT_ARR_ALLOC(miniexact_name, name)
  MINIEXACT_ARR_ALLOC(miniexact_name, color_name)
  MINIEXACT_ARR_ALLOC(miniexact_name, len)
  MINIEXACT_ARR_ALLOC(miniexact_name, ulink)
  MINIEXACT_ARR_ALLOC(miniexact_name, dlink)
  MINIEXACT_ARR_ALLOC(miniexact_name, x)
  MINIEXACT_ARR_ALLOC(miniexact_color, color)
  MINIEXACT_ARR_ALLOC(miniexact_link, ft)
  MINIEXACT_ARR_ALLOC(miniexact_link, slack)
  MINIEXACT_ARR_ALLOC(miniexact_link, bound)
  MINIEXACT_ARR_ALLOC(uint32_t, cost)
  MINIEXACT_ARR_ALLOC(uint32_t, best)
  MINIEXACT_ARR_ALLOC(uint32_t, tho)
  MINIEXACT_ARR_ALLOC(uint32_t, th)

  LLINK(0) = 0;
  RLINK(0) = 0;
  NAME(0) = NULL;
  MINIEXACT_ARR_PLUS1(color_name)
  p->color_name[0] = NULL;

  p->name_size = 1;
  p->llink_size = 1;
  p->rlink_size = 1;

  p->i = 0;
  p->j = 0;
  p->N_1 = -1;

  p->state = 0;

  p->longest_option = 0;

  return NULL;
}

miniexact_link
miniexact_choose_i_naively(miniexact_algorithm* a,
                           miniexact_problem* p,
                           int32_t t) {
  (void)t;
  return RLINK(0);
}

miniexact_link
miniexact_choose_i_naively_cost(miniexact_algorithm* a,
                                miniexact_problem* p,
                                int32_t t) {
  return RLINK(0);
}

miniexact_link
miniexact_choose_i_mrv(miniexact_algorithm* a,
                       miniexact_problem* p,
                       int32_t t) {
  (void)t;
  miniexact_link i = RLINK(0);
  miniexact_link p_ = RLINK(0), theta = MINIEXACT_LINK_MAX;
  while(p_ != 0) {
    miniexact_link lambda = LEN(p_);
    if(lambda < theta) {
      theta = lambda;
      i = p_;
    }
    if(lambda == 0) {
      return i;
    }
    p_ = RLINK(p_);
  }
  return i;
}

miniexact_link
miniexact_choose_i_mrv_cost(miniexact_algorithm* a,
                            miniexact_problem* p,
                            int32_t cutoff) {
  // This implementation stems from the solution to exercise 248,
  // 7.2.2.1 (Page 288, Fascicle 4).

  const int32_t L = 10; // Magic from Knuth.
  int32_t t = INT32_MAX;
  int32_t c = 0;
  miniexact_link j = RLINK(0);
  miniexact_link i = j;
  while(j > 0) {
    miniexact_link s = 0;
    miniexact_link p_ = DLINK(j);
    int32_t c_prime = COST(p_);
    if(p_ == j || c_prime >= cutoff) {
      return -1;
    } else {
      s = 1;
      p_ = DLINK(p_);
      for(;;) {
	if(p_ == j || COST(p_) >= cutoff)
	  break;
	else if(s == t) {
	  s = s + 1;
	  break;
	} else if(s >= L) {
	  s = LEN(p->l);
	  break;
	} else {
	  s = s + 1;
	  p_ = DLINK(p_);
	}
      }
      if(s < t || (s == t && c < c_prime)) {
	t = s;
	i = j;
	c = c_prime;
      }
      j = RLINK(j);
    }
  }
  return i;
}

miniexact_link
miniexact_choose_i_mrv_slacker(miniexact_algorithm* a,
                               miniexact_problem* p,
                               int32_t t) {
  miniexact_link theta = MINIEXACT_LINK_MAX;
  miniexact_link i = RLINK(0);
  miniexact_link p_ = RLINK(0);
  while(p_ != 0) {
    miniexact_link lambda = THETA(p_);
    if(lambda < theta || (lambda == theta && SLACK(p_) < SLACK(i)) ||
       (lambda == theta && SLACK(p_) == SLACK(i) && LEN(p_) > LEN(i))) {
      theta = lambda;
      i = p_;
      assert(i <= p->primary_item_count);
    }
    p_ = RLINK(p_);
  }
  assert(i <= p->primary_item_count);
  return i;
}

void
miniexact_algorithm_standard_functions(miniexact_algorithm* a) {
  a->add_item = &add_item;
  a->add_item_with_color = &add_item_with_color;
  a->prepare_options = &prepare_options;
  a->end_option = &end_option;
  a->define_primary_item = &define_primary_item;
  a->define_primary_item_with_range = &define_primary_item_with_range;
  a->define_secondary_item = &define_secondary_item;
  a->end_options = &end_options;
  a->init_problem = &miniexact_default_init_problem;

  // Default: Just use MRV.
  a->choose_i = &miniexact_choose_i_mrv;

  // Nothing to be freed by default.
  a->free_userdata = NULL;
}

bool
miniexact_algorithm_from_select(int algorithm_select,
                                miniexact_algorithm* algorithm) {
  bool success = false;
  if(algorithm_select & MINIEXACT_ALGORITHM_X) {
    miniexact_algorithm_x_set(algorithm);
    success = true;
  } else if(algorithm_select & MINIEXACT_ALGORITHM_C) {
    miniexact_algorithm_c_set(algorithm);
    success = true;
  } else if(algorithm_select & MINIEXACT_ALGORITHM_C_DOLLAR) {
    miniexact_algorithm_c_dollar_set(algorithm);
    success = true;
  } else if(algorithm_select & MINIEXACT_ALGORITHM_M) {
    miniexact_algorithm_m_set(algorithm);
    // Set default for Algorithm M. May be overriden, as it is later the first
    // to be checked.
    algorithm_select |= MINIEXACT_ALGORITHM_MRV_SLACKER;
    success = true;
#ifdef MINIEXACT_SAT_SOLVER_AVAILABLE
  } else if(algorithm_select & MINIEXACT_ALGORITHM_KNUTH_CNF) {
    miniexact_algoritihm_knuth_cnf_set(algorithm);
    success = true;
#endif
  }

  if(algorithm_select & MINIEXACT_ALGORITHM_DOLLARS) {
    if(algorithm_select & MINIEXACT_ALGORITHM_NAIVE) {
      algorithm->choose_i = &miniexact_choose_i_naively_cost;
    }
    if(algorithm_select & MINIEXACT_ALGORITHM_MRV) {
      algorithm->choose_i = &miniexact_choose_i_mrv_cost;
    }
  } else {
    if(algorithm_select & MINIEXACT_ALGORITHM_MRV_SLACKER) {
      algorithm->choose_i = &miniexact_choose_i_mrv_slacker;
    }
    if(algorithm_select & MINIEXACT_ALGORITHM_NAIVE) {
      algorithm->choose_i = &miniexact_choose_i_naively;
    }
    if(algorithm_select & MINIEXACT_ALGORITHM_MRV) {
      algorithm->choose_i = &miniexact_choose_i_mrv;
    }
  }

  return success;
}

struct miniexact_algorithm*
miniexact_algorithm_allocate() {
  return calloc(1, sizeof(miniexact_algorithm));
}

miniexact_algorithm*
miniexact_algorithm_x_allocate() {
  miniexact_algorithm* a = miniexact_algorithm_allocate();
  miniexact_algorithm_x_set(a);
  return a;
}

miniexact_algorithm*
miniexact_algorithm_c_allocate() {
  miniexact_algorithm* a = miniexact_algorithm_allocate();
  miniexact_algorithm_c_set(a);
  return a;
}

miniexact_algorithm*
miniexact_algorithm_m_allocate() {
  miniexact_algorithm* a = miniexact_algorithm_allocate();
  miniexact_algorithm_m_set(a);
  return a;
}

miniexact_algorithm*
miniexact_algorithm_c_dollar_allocate() {
  miniexact_algorithm* a = miniexact_algorithm_allocate();
  miniexact_algorithm_c_dollar_set(a);
  return a;
}
