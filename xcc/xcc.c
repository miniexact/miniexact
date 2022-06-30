#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "xcc.h"

xcc_problem*
xcc_problem_allocate() {
  return calloc(1, sizeof(xcc_problem));
}

int
xcc_search_for_name(xcc_name needle, xcc_name* names, size_t names_size) {
  for(size_t i = 0; i < names_size; ++i)
    if(names[i] && strcmp(names[i], needle) == 0)
      return i;
  return -1;
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
  if(p->name) {
    for(size_t i = 0; i < p->name_size; ++i)
      if(p->name[i])
	free(p->name[i]);
    free(p->name);
  }

  free(p);
}

xcc_link
xcc_item_from_ident(xcc_problem* p, const char* ident) {
  return 1;
}
