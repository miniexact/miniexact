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

#define XCC_ARR_PLUSN(ARR, N)                     \
  while(p->ARR##_size + N >= p->ARR##_capacity) { \
    XCC_ARR_REALLOC(ARR)                          \
  }                                               \
  p->ARR##_size += N;

#define XCC_ARR_PLUS1(ARR) XCC_ARR_PLUSN(ARR, 1)

typedef struct xcc_problem {
  ARR(xcc_link, llink)
  ARR(xcc_link, rlink)
  ARR(xcc_link, ulink)
  ARR(xcc_link, dlink)
  union {
    struct {
      ARR(xcc_link, top)
    };
    struct {
      ARR(xcc_link, len)
    };
  };

  ARR(xcc_name, name)

  // Solution
  ARR(xcc_link, x)

  int N, N_1, M, i, j, p, q, Z;
  int primary_item_count;
  int secondary_item_count;
  int option_count;
} xcc_problem;

#undef ARR

int
xcc_search_for_name(const xcc_name needle,
                    const xcc_name* names,
                    size_t names_size);

bool
xcc_has_item(xcc_link needle, xcc_link* list, size_t len);

xcc_problem*
xcc_problem_allocate();

void
xcc_problem_free(xcc_problem* p);

xcc_link
xcc_item_from_ident(xcc_problem* p, xcc_name ident);

void
xcc_print_problem_matrix(xcc_problem* p);

#endif
