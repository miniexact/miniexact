/*
    XCCSolve - Toolset to solve exact cover problems and extensions
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
#include "xcc/xcc.h"
#include <xcc/algorithm.h>
#include <xcc/algorithm_c.h>
#include <xcc/algorithm_knuth_cnf.h>
#include <xcc/algorithm_m.h>
#include <xcc/algorithm_x.h>
#include <xcc/ops.h>

static inline const char*
define_item(xcc_algorithm* a, xcc_problem* p, xcc_link l) {
  assert(a);
  assert(p);
  assert(l);
  assert(p->llink);
  assert(p->rlink);

  XCC_ARR_PLUS1(llink)
  XCC_ARR_PLUS1(rlink)

  p->i = p->i + 1;
  LLINK(p->i) = p->i - 1;
  RLINK(p->i - 1) = p->i;

  return NULL;
}

static const char*
define_primary_item(xcc_algorithm* a, xcc_problem* p, xcc_link l) {
  const char* e;
  if((e = define_item(a, p, l)))
    return e;

  ++p->primary_item_count;

  // Always track them with as the base-case w.r.t. primary items.
  XCC_ARR_PLUS1(slack)
  XCC_ARR_PLUS1(bound)
  SLACK(p->i) = 1;
  BOUND(p->i) = 0;

  return NULL;
}

// Adds a primary item with a range [u;v] (sets SLACK and BOUND)
static const char*
define_primary_item_with_range(xcc_algorithm* a,
                               xcc_problem* p,
                               xcc_link l,
                               xcc_link u,
                               xcc_link v) {
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

  XCC_ARR_PLUS1(slack)
  XCC_ARR_PLUS1(bound)

  SLACK(p->i) = v - u;
  BOUND(p->i) = v;

  return NULL;
}

static const char*
define_secondary_item(xcc_algorithm* a, xcc_problem* p, xcc_link l) {
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
prepare_options(xcc_algorithm* a, xcc_problem* p) {
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
  XCC_ARR_PLUSN(len, p->N + 2);
  XCC_ARR_PLUSN(ulink, p->N + 2);
  XCC_ARR_PLUSN(dlink, p->N + 2);

  // Normalize the don't cares
  ULINK(p->N + 1) = 0;
  DLINK(p->N + 1) = 0;

  LEN(0) = 0;
  ULINK(0) = 0;
  DLINK(0) = 0;

  for(int i = 1; i <= p->N; ++i) {
    LEN(i) = 0;
    ULINK(i) = i;
    DLINK(i) = i;
  }

  p->M = 0;
  p->p = p->N + 1;
  TOP(p->p) = 0;

  p->Z = p->p;
  return NULL;
}

static const char*
add_item_with_color(xcc_algorithm* a,
                    xcc_problem* p,
                    xcc_link ij,
                    xcc_color c) {
  if(ij < 1)
    return "Invalid ij given for add_item!";

  XCC_ARR_PLUS1(len)
  XCC_ARR_PLUS1(dlink)
  XCC_ARR_PLUS1(ulink)
  XCC_ARR_PLUS1(color)

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
add_item(xcc_algorithm* a, xcc_problem* p, xcc_link ij) {
  return add_item_with_color(a, p, ij, 0);
}

static const char*
end_option(xcc_algorithm* a, xcc_problem* p) {
  XCC_ARR_PLUS1(len)
  XCC_ARR_PLUS1(dlink)
  XCC_ARR_PLUS1(ulink)

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
end_options(xcc_algorithm* a, xcc_problem* p) {
  DLINK(p->dlink_size - 1) = 0;
  return NULL;
}

const char*
xcc_default_init_problem(xcc_algorithm* a, xcc_problem* p) {
  assert(a);
  assert(p);

  p->algorithm_userdata = NULL;

  XCC_ARR_ALLOC(xcc_link, llink)
  XCC_ARR_ALLOC(xcc_link, rlink)
  XCC_ARR_ALLOC(xcc_name, name)
  XCC_ARR_ALLOC(xcc_name, color_name)
  XCC_ARR_ALLOC(xcc_name, len)
  XCC_ARR_ALLOC(xcc_name, ulink)
  XCC_ARR_ALLOC(xcc_name, dlink)
  XCC_ARR_ALLOC(xcc_name, x)
  XCC_ARR_ALLOC(xcc_color, color)
  XCC_ARR_ALLOC(xcc_link, ft)
  XCC_ARR_ALLOC(xcc_link, slack)
  XCC_ARR_ALLOC(xcc_link, bound)

  LLINK(0) = 0;
  RLINK(0) = 0;
  NAME(0) = NULL;
  p->color_name[0] = NULL;
  XCC_ARR_PLUS1(color_name)

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

xcc_link
xcc_choose_i_naively(xcc_algorithm* a, xcc_problem* p) {
  return RLINK(0);
}

xcc_link
xcc_choose_i_mrv(xcc_algorithm* a, xcc_problem* p) {
  xcc_link i = RLINK(0);
  xcc_link p_ = RLINK(0), theta = XCC_LINK_MAX;
  while(p_ != 0) {
    xcc_link lambda = LEN(p_);
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

xcc_link
xcc_choose_i_mrv_slacker(xcc_algorithm* a, xcc_problem* p) {
  xcc_link theta = XCC_LINK_MAX;
  xcc_link i = RLINK(0);
  xcc_link p_ = RLINK(0);
  while(p_ != 0) {
    xcc_link lambda = THETA(p_);
    if(lambda < theta || (lambda == theta && SLACK(p_) == SLACK(i)) ||
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
xcc_algorithm_standard_functions(xcc_algorithm* a) {
  a->add_item = &add_item;
  a->add_item_with_color = &add_item_with_color;
  a->prepare_options = &prepare_options;
  a->end_option = &end_option;
  a->define_primary_item = &define_primary_item;
  a->define_primary_item_with_range = &define_primary_item_with_range;
  a->define_secondary_item = &define_secondary_item;
  a->end_options = &end_options;
  a->init_problem = &xcc_default_init_problem;

  // Default: Just use MRV.
  a->choose_i = &xcc_choose_i_mrv;

  // Nothing to be freed by default.
  a->free_userdata = NULL;
}

bool
xcc_algorithm_from_select(int algorithm_select, xcc_algorithm* algorithm) {
  bool success = false;
  if(algorithm_select & XCC_ALGORITHM_X) {
    xcc_algorithm_x_set(algorithm);
    success = true;
  } else if(algorithm_select & XCC_ALGORITHM_C) {
    xcc_algorithm_c_set(algorithm);
    success = true;
  } else if(algorithm_select & XCC_ALGORITHM_M) {
    xcc_algorithm_m_set(algorithm);
    // Set default for Algorithm M. May be overriden, as it is later the first
    // to be checked.
    algorithm_select |= XCC_ALGORITHM_MRV_SLACKER;
    success = true;
#ifdef XCC_SAT_SOLVER_AVAILABLE
  } else if(algorithm_select & XCC_ALGORITHM_KNUTH_CNF) {
    xcc_algoritihm_knuth_cnf_set(algorithm);
    success = true;
#endif
  }

  if(algorithm_select & XCC_ALGORITHM_MRV_SLACKER) {
    algorithm->choose_i = &xcc_choose_i_mrv_slacker;
  }
  if(algorithm_select & XCC_ALGORITHM_NAIVE) {
    algorithm->choose_i = &xcc_choose_i_naively;
  }
  if(algorithm_select & XCC_ALGORITHM_MRV) {
    algorithm->choose_i = &xcc_choose_i_mrv;
  }

  return success;
}

struct xcc_algorithm*
xcc_algorithm_allocate() {
  return calloc(1, sizeof(xcc_algorithm));
}

xcc_algorithm*
xcc_algorithm_x_allocate() {
  xcc_algorithm* a = xcc_algorithm_allocate();
  xcc_algorithm_x_set(a);
  return a;
}

xcc_algorithm*
xcc_algorithm_c_allocate() {
  xcc_algorithm* a = xcc_algorithm_allocate();
  xcc_algorithm_c_set(a);
  return a;
}

xcc_algorithm*
xcc_algorithm_m_allocate() {
  xcc_algorithm* a = xcc_algorithm_allocate();
  xcc_algorithm_m_set(a);
  return a;
}
