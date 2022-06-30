#ifndef XCC_H
#define XCC_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef int32_t xcc_link;
typedef xcc_link xcc_color;
typedef char* xcc_name;
typedef struct xcc_node {
  xcc_link ulink, dlink;
  xcc_color color;
} xcc_node;

#define ARR(TYPE, NAME) \
  TYPE* NAME;           \
  size_t NAME##_size;   \
  size_t NAME##_capacity;

#define XCC_ARR_ALLOC(TYPE, ARR)                     \
  p->ARR##_capacity = 65536;                         \
  p->ARR = malloc(p->ARR##_capacity * sizeof(TYPE)); \
  p->ARR##_size = 0;

#define XCC_ARR_REALLOC(ARR)                 \
  p->ARR##_capacity = p->ARR##_capacity * 4; \
  p->ARR = realloc(p->ARR, p->ARR##_capacity * sizeof(p->ARR[0]));

typedef struct xcc_problem {
  ARR(xcc_link, llink)
  ARR(xcc_link, rlink)
  ARR(xcc_link, ulink)
  ARR(xcc_link, dlink)
  ARR(xcc_link, top)
  ARR(xcc_name, name)
} xcc_problem;

#undef ARR

xcc_problem*
xcc_problem_allocate();

void
xcc_problem_free(xcc_problem* p);

xcc_link
xcc_item_from_ident(xcc_problem* p, const char* ident);

#endif
