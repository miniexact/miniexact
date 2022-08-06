#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <cadical/ccadical.h>

#include "algorithm.h"
#include "algorithm_knuth_cnf.h"
#include "log.h"
#include "ops.h"

struct algorithm_knuth_cnf {
  CCaDiCaL* cadical;
};

static struct algorithm_knuth_cnf*
create_k() {
  struct algorithm_knuth_cnf* k = malloc(sizeof(struct algorithm_knuth_cnf));
  k->cadical = ccadical_init();
  return k;
}

static inline void
unit(CCaDiCaL* c, int l) {
  ccadical_add(c, l);
  ccadical_add(c, 0);
}

static inline void
binary(CCaDiCaL* c, int a, int b) {
  ccadical_add(c, a);
  ccadical_add(c, b);
  ccadical_add(c, 0);
}

static inline void
ternary(CCaDiCaL* c_, int a, int b, int c) {
  ccadical_add(c_, a);
  ccadical_add(c_, b);
  ccadical_add(c_, c);
  ccadical_add(c_, 0);
}

static inline xcc_link
get_option_id(xcc_problem* p, xcc_link i) {
  while(TOP(i) > 0)
    ++i;
  return -TOP(i);
}

static void
encode_problem(xcc_problem* p) {
  struct algorithm_knuth_cnf* k = p->algorithm_userdata;
  CCaDiCaL* c = k->cadical;

  if(p->N != p->N_1) {
    err("No secondary items supported by the SAT solver (yet)!");
    return;
  }

  // Go downwards from items so that every option is captured.
  for(xcc_link i = 1; i <= p->N_1; ++i) {
    for(xcc_link down = DLINK(i); down > ULINK(down); down = DLINK(down)) {
      xcc_link option = get_option_id(p, down);
      ccadical_add(c, option);
    }
    ccadical_add(c, 0);
    for(xcc_link down1 = DLINK(i); down1 > ULINK(down1); down1 = DLINK(down1)) {
      xcc_link option1 = get_option_id(p, down1);
      for(xcc_link down2 = DLINK(i); down2 > ULINK(down2);
          down2 = DLINK(down2)) {
        if(down1 == down2)
          continue;
        xcc_link option2 = get_option_id(p, down2);
        binary(c, -option1, -option2);
      }
    }
  }
}

static bool
compute_next_result(xcc_algorithm* a, xcc_problem* p) {
  if(!p->algorithm_userdata) {
    p->algorithm_userdata = create_k();
    encode_problem(p);
  }
  struct algorithm_knuth_cnf* k = p->algorithm_userdata;

  if(p->x_size != 0) {
    // Encode previous result! Not that again.
    xcc_link options[p->x_size];
    xcc_extract_solution_option_indices(p, options);
    CCaDiCaL* c = k->cadical;
    for(size_t i = 0; i < p->x_size; ++i) {
      ccadical_add(c, -options[i]);
    }
    ccadical_add(c, 0);
  }

  int r = ccadical_solve(k->cadical);

  if(r == 20)
    return false;
  else if(r == 10) {
    p->x_size = 0;
    for(xcc_link i = p->N + 2; i <= p->Z; ++i) {
      xcc_link t = TOP(i);
      if(t < 0) {
        bool active = ccadical_val(k->cadical, -t) > 0;
        if(active) {
          p->x[p->x_size] = i - 1;
          XCC_ARR_PLUS1(x)
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
  ccadical_release(k->cadical);
  k->cadical = NULL;

  free(k);
}

void
xcc_algoritihm_knuth_cnf_set(xcc_algorithm* a) {
  xcc_algorithm_standard_functions(a);

  a->compute_next_result = &compute_next_result;
  a->free_userdata = &free_userdata;
}
