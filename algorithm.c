#include "algorithm.h"
#include "algorithm_x.h"
#include "ops.h"

xcc_link
xcc_choose_i_naively(xcc_algorithm* a, xcc_problem* p) {
  return RLINK(0);
}

xcc_link
xcc_choose_i_mrv(xcc_algorithm* a, xcc_problem* p) {
  xcc_link i = xcc_choose_i_naively(a, p);

  xcc_link p_ = RLINK(0);
  xcc_link theta = XCC_LINK_MAX;
  while(p_ != 0) {
    xcc_link lambda = LEN(p->p);
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

bool
xcc_algorithm_from_select(int algorithm_select, xcc_algorithm* algorithm) {
  if(algorithm_select & XCC_ALGORITHM_X) {
    xcc_algoritihm_x_set(algorithm);
    return true;
  }

  return false;
}
