#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "algorithm.h"
#include "algorithm_x.h"

static inline bool
define_item(xcc_algorithm* a, xcc_problem* p, xcc_name n) {
  assert(a);
  assert(p);
  assert(n);
  assert(p->name);
  assert(p->llink);
  assert(p->rlink);

  int found = 0;
  if((found = xcc_search_for_name(n, p->name, p->name_size - 1) >= 0)) {
    fprintf(stderr, "Name %s already defined as item number %d!\n", n, found);
    return false;
  }

  XCC_ARR_PLUS1(name)
  XCC_ARR_PLUS1(llink)
  XCC_ARR_PLUS1(rlink)

  p->i = p->i + 1;
  p->name[p->i] = strdup(n);
  p->llink[p->i] = p->i - 1;
  p->rlink[p->i - 1] = p->i;

  return true;
}

static bool
define_primary_item(xcc_algorithm* a, xcc_problem* p, xcc_name n) {
  if(!define_item(a, p, n))
    return false;

  ++p->primary_item_count;

  return true;
}

static bool
define_secondary_item(xcc_algorithm* a, xcc_problem* p, xcc_name n) {
  if(!define_item(a, p, n))
    return false;

  ++p->secondary_item_count;

  return true;
}

static bool
prepare_options(xcc_algorithm* a, xcc_problem* p) {
  // Step I2
  p->N = p->i;
  if(p->N_1 < 0)
    p->N_1 = p->N;
  p->llink[p->N + 1] = p->N;
  p->rlink[p->N] = p->N + 1;
  p->llink[p->N_1 + 1] = p->N + 1;
  p->rlink[p->N + 1] = p->N_1 + 1;
  p->llink[0] = p->N_1;
  p->rlink[p->N_1] = 0;

  // Step N3
  XCC_ARR_PLUSN(len, p->N)
  XCC_ARR_PLUSN(ulink, p->N)
  XCC_ARR_PLUSN(dlink, p->N)
  for(int i = 1; i <= p->N; ++i) {
    p->len[i] = 0;
    p->ulink[i] = i;
    p->dlink[i] = i;
  }
  return true;
}

static bool
add_item(xcc_algorithm* a, xcc_problem* p, xcc_link l) {
  return true;
}

static bool
end_option(xcc_algorithm* a, xcc_problem* p) {
  return true;
}

static bool
init_problem(xcc_algorithm* a, xcc_problem* p) {
  assert(a);
  assert(p);

  XCC_ARR_ALLOC(xcc_link, llink)
  XCC_ARR_ALLOC(xcc_link, rlink)
  XCC_ARR_ALLOC(xcc_name, name)
  XCC_ARR_ALLOC(xcc_name, len)
  XCC_ARR_ALLOC(xcc_name, ulink)
  XCC_ARR_ALLOC(xcc_name, dlink)

  p->llink[0] = 0;
  p->rlink[0] = 0;
  p->name[0] = NULL;

  p->name_size = 1;
  p->llink_size = 1;
  p->rlink_size = 1;

  p->i = 0;
  p->N_1 = -1;
  return true;
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
}
