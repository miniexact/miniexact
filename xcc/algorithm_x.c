#include <stdlib.h>

#include "algorithm.h"
#include "algorithm_x.h"

static bool
add_item(xcc_algorithm* a, xcc_problem* p, xcc_link l) {
  return true;
}

static bool
define_primary_item(xcc_algorithm* a, xcc_problem* p, xcc_name n) {
  return true;
}

static bool
define_secondary_item(xcc_algorithm* a, xcc_problem* p, xcc_name n) {
  return false;
}

static bool
end_option(xcc_algorithm* a, xcc_problem* p) {
  return true;
}

static bool
init_problem(xcc_algorithm* a, xcc_problem* p) {
  XCC_ARR_ALLOC(xcc_link, llink)
  XCC_ARR_ALLOC(xcc_link, rlink)
}

void
xcc_algoritihm_x_set(xcc_algorithm* a) {
  a->add_item = &add_item;
  a->add_item_with_color = NULL;
  a->end_option = &end_option;

  a->define_primary_item = &define_primary_item;
  a->define_secondary_item = &define_secondary_item;

  a->init_problem = &init_problem;
}
