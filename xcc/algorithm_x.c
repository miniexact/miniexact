#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "algorithm.h"
#include "algorithm_x.h"

#include "ops.h"

typedef enum x_state { X1, X2, X3, X4, X5, X6, X7, X8 } x_state;

static inline const char*
define_item(xcc_algorithm* a, xcc_problem* p, xcc_name n) {
  assert(a);
  assert(p);
  assert(n);
  assert(p->name);
  assert(p->llink);
  assert(p->rlink);

  int found = 0;
  if((found = xcc_search_for_name(n, p->name, p->name_size - 1) >= 0)) {
    return "Name already defined as item!";
  }

  XCC_ARR_PLUS1(name)
  XCC_ARR_PLUS1(llink)
  XCC_ARR_PLUS1(rlink)

  p->i = p->i + 1;
  NAME(p->i) = n;
  LLINK(p->i) = p->i - 1;
  RLINK(p->i - 1) = p->i;

  return NULL;
}

static const char*
define_primary_item(xcc_algorithm* a, xcc_problem* p, xcc_name n) {
  const char* e;
  if((e = define_item(a, p, n)))
    return e;

  ++p->primary_item_count;

  return NULL;
}

static const char*
define_secondary_item(xcc_algorithm* a, xcc_problem* p, xcc_name n) {
  const char* e;
  if((e = define_item(a, p, n)))
    return e;

  ++p->secondary_item_count;

  return NULL;
  ;
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
  XCC_ARR_PLUSN(len, p->N + 2)
  XCC_ARR_PLUSN(ulink, p->N + 2)
  XCC_ARR_PLUSN(dlink, p->N + 2)

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
add_item(xcc_algorithm* a, xcc_problem* p, xcc_link ij) {
  if(ij < 1)
    return "Invalid ij given for add_item!";

  XCC_ARR_PLUS1(len)
  XCC_ARR_PLUS1(dlink)
  XCC_ARR_PLUS1(ulink)

  ++p->j;

  LEN(ij) = LEN(ij) + 1;
  p->q = ULINK(ij);
  ULINK(p->p + p->j) = p->q;
  DLINK(p->q) = p->p + p->j;
  DLINK(p->p + p->j) = ij;
  ULINK(ij) = p->p + p->j;
  TOP(p->p + p->j) = ij;

  return NULL;
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

  p->j = 0;
  p->Z = p->p;

  return NULL;
}

static const char*
init_problem(xcc_algorithm* a, xcc_problem* p) {
  assert(a);
  assert(p);

  XCC_ARR_ALLOC(xcc_link, llink)
  XCC_ARR_ALLOC(xcc_link, rlink)
  XCC_ARR_ALLOC(xcc_name, name)
  XCC_ARR_ALLOC(xcc_name, len)
  XCC_ARR_ALLOC(xcc_name, ulink)
  XCC_ARR_ALLOC(xcc_name, dlink)
  XCC_ARR_ALLOC(xcc_name, x)

  LLINK(0) = 0;
  RLINK(0) = 0;
  NAME(0) = NULL;

  p->name_size = 1;
  p->llink_size = 1;
  p->rlink_size = 1;

  p->i = 0;
  p->j = 0;
  p->N_1 = -1;

  p->state = X1;

  return NULL;
}

static bool
compute_next_result(xcc_algorithm* a, xcc_problem* p) {
  if(p->x_capacity < p->option_count) {
    p->x = realloc(p->x, sizeof(xcc_link) * p->option_count);
    p->x_capacity = p->option_count;
    p->x_size = 0;
  }

  assert(a->choose_i);

  while(true) {
    switch(p->state) {
      case X1:
        p->l = 0;
        p->state = X2;
        break;
      case X2:
        if(RLINK(0) == 0) {
          p->state = X8;
	  p->x_size = p->l;
          return true;
        }
        p->state = X3;
        break;
      case X3:
        p->i = a->choose_i(a, p);
        p->state = X4;
        break;
      case X4:
        COVER(p->i);
        p->x[p->l] = DLINK(p->i);
        p->state = X5;
        break;
      case X5:
        if(p->x[p->l] == p->i) {
          p->state = X7;
        } else {
          p->p = p->x[p->l] + 1;
          while(p->p != p->x[p->l]) {
            xcc_link j = TOP(p->p);
            if(j <= 0) {
              p->p = ULINK(p->p);
            } else {
              COVER(j);
              p->p = p->p + 1;
            }
          }
        }
        p->l = p->l + 1;
        p->state = X2;
        break;
      case X6:
        p->p = p->x[p->l] - 1;
        while(p->p != p->x[p->l]) {
          xcc_link j = TOP(p->p);
          if(j <= 0) {
            p->p = DLINK(p->p);
          } else {
            UNCOVER(j);
            p->p = p->p - 1;
          }
        }
        p->i = TOP(p->x[p->l]);
        p->x[p->l] = DLINK(p->x[p->l]);
        p->state = X5;
        break;
      case X7:
        UNCOVER(p->i);
        p->state = X8;
        break;
      case X8:
        if(p->l == 0) {
          return false;
        }
        p->l = p->l - 1;
        p->state = X6;
        break;
    }
  }

  return false;
}

void
xcc_algoritihm_x_set(xcc_algorithm* a) {
  a->add_item = &add_item;
  a->add_item_with_color = NULL;
  a->prepare_options = &prepare_options;
  a->end_option = &end_option;

  a->define_primary_item = &define_primary_item;
  a->define_secondary_item = &define_secondary_item;

  a->init_problem = &init_problem;
  a->compute_next_result = &compute_next_result;
  a->choose_i = &xcc_choose_i_mrv;
}
