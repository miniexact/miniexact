#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <xcc/algorithm.h>
#include <xcc/ops.h>
#include <xcc/xcc.h>

xcc_problem*
xcc_problem_allocate() {
  return calloc(1, sizeof(xcc_problem));
}

int
xcc_search_for_name(const xcc_name needle,
                    const xcc_name* names,
                    size_t names_size) {
  for(int i = 0; i < names_size; ++i) {
    if(names[i] && strcmp(names[i], needle) == 0) {
      return i;
    }
  }
  return -1;
}

bool
xcc_has_item(xcc_link needle, xcc_link* list, size_t len) {
  for(size_t i = 0; i < len; ++i) {
    if(list[i] == needle)
      return true;
  }
  return false;
}

void
xcc_problem_free(xcc_problem* p, xcc_algorithm* a) {
  if(p->algorithm_userdata && a && a->free_userdata)
    a->free_userdata(a, p);

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
  if(p->color)
    free(p->color);
  if(p->color_name) {
    for(size_t i = 0; i < p->color_name_size; ++i)
      if(p->color_name[i])
        free(p->color_name[i]);
    free(p->color_name);
  }
  if(p->name) {
    for(size_t i = 0; i < p->name_size; ++i)
      if(p->name[i])
        free(p->name[i]);
    free(p->name);
  }
  if(p->x)
    free(p->x);

  free(p);
}

xcc_link
xcc_item_from_ident(xcc_problem* p, xcc_name ident) {
  return xcc_search_for_name(ident, p->name, p->name_size);
}

xcc_link
xcc_color_from_ident(xcc_problem* p, xcc_name ident) {
  return xcc_search_for_name(ident, p->color_name, p->color_name_size);
}

xcc_link
xcc_color_from_ident_and_insert(xcc_problem* p, xcc_name ident) {
  xcc_link l = xcc_color_from_ident(p, ident);
  if(l == -1) {
    l = p->color_name_size;
    XCC_ARR_PLUS1(color_name)
    p->color_name[l] = ident;
  } else {
    free(ident);
  }
  return l;
}

void
xcc_print_problem_matrix(xcc_problem* p) {
  // Print Header first
  for(size_t i = 0; i < p->name_size; ++i) {
    printf("i:%zu\tNAME:%s\tLLINK:%d\tRLINK:%d\n",
           i,
           p->name[i],
           p->llink[i],
           p->rlink[i]);
  }
  printf("\n");
  for(size_t x = 0; x < p->ulink_size; ++x) {
    printf("x:%zu\tTOP/LEN:%d\tULINK:%d\tDLINK:%d\n",
           x,
           p->top[x],
           p->ulink[x],
           p->dlink[x]);
  }
}

void
xcc_print_problem_solution(xcc_problem* p) {
  printf("Solution:\n");
  for(size_t i = 0; i < p->x_size; ++i) {
    printf("%d ", p->x[i]);
  }
  printf("\n");
}

void
xcc_extract_solution_option_indices(xcc_problem* p, xcc_link* solution) {
  assert(p);
  assert(solution);

  for(xcc_link j = 0; j < p->l; ++j) {
    xcc_link r = p->x[j];
    while(TOP(r) >= 0)
      ++r;
    solution[j] = -TOP(r);
  }
}
