#include <stdlib.h>

#include "xcc.h"

xcc_problem*
xcc_problem_allocate() {
  return malloc(sizeof(xcc_problem));
}

void
xcc_problem_free(xcc_problem* p) {
  free(p);
}

xcc_link
xcc_item_from_ident(xcc_problem* p, const char* ident) {
  return 1;
}
