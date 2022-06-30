#include <stdlib.h>

#include "algorithm.h"
#include "algorithm_x.h"

static bool
add_option(xcc_algorithm* a, xcc_problem* p, xcc_link l) {
  return false;
}

static bool
define_primary_item(xcc_algorithm* a, xcc_problem* p, xcc_name n) {
  return false;
}

static bool
define_secondary_item(xcc_algorithm* a, xcc_problem* p, xcc_name n) {
  return false;
}

void
xcc_algoritihm_x_set(xcc_algorithm* a) {
  a->add_option = &add_option;
  a->add_option_with_color = NULL;

  a->define_primary_item = &define_primary_item;
  a->define_secondary_item = &define_secondary_item;
}
