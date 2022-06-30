#include <stdlib.h>
#include <assert.h>

#include "xcc.h"

xcc_problem*
xcc_problem_allocate() {
  return calloc(1, sizeof(xcc_problem));
}

void
xcc_problem_free(xcc_problem* p) {
  if(p->llink)
    free(p->llink);
  if(p->rlink)
    free(p->rlink);
  if(p->ulink)
    free(p->ulink);
  if(p->dlink)
    free(p->dlink);
  if(p->top)
    free(p->top);
  if(p->name)
    free(p->name);

  free(p);
}

xcc_link
xcc_item_from_ident(xcc_problem* p, const char* ident) {
  return 1;
}
