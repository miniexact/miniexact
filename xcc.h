#ifndef XCC_H
#define XCC_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef int32_t xcc_link;
typedef xcc_link xcc_color;
typedef char* xcc_name;
typedef struct xcc_algorithm xcc_algorithm;

#define XCC_LINK_MAX INT32_MAX

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

typedef struct xcc_config {
  int verbose;
  int print_options;
  int enumerate;
  int algorithm_select;
  const char* input_file;
} xcc_config;

typedef enum xcc_algorithm_id {
  XCC_ALGORITHM_NAIVE = 1 << 1,
  XCC_ALGORITHM_MRV = 1 << 2,
  XCC_ALGORITHM_X = 1 << 3,
  XCC_ALGORITHM_C = 1 << 4,
  XCC_ALGORITHM_KNUTH_CNF = 1 << 5
} xcc_algorithm_id;

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
  ARR(xcc_name, color_name)
  ARR(xcc_color, color)

  // Solution
  ARR(xcc_link, x)

  int N, N_1, M, i, j, l, p, q, Z;
  int primary_item_count;
  int secondary_item_count;
  int option_count;
  int state;

  void* algorithm_userdata;
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
xcc_problem_free(xcc_problem* p, xcc_algorithm* a);

xcc_link
xcc_item_from_ident(xcc_problem* p, xcc_name ident);

xcc_link
xcc_color_from_ident(xcc_problem* p, xcc_name ident);

xcc_link
xcc_color_from_ident_and_insert(xcc_problem* p, xcc_name ident);

void
xcc_print_problem_matrix(xcc_problem* p);

void
xcc_print_problem_solution(xcc_problem* p);

/** @brief Extract a valid solution, consisting of the option indices
 *
 * Requires p to be in a valid solved state and solution to be a pointer to an
 * array with at least p->l elements.
 *
 * This is basically exercise 13.
 */
void
xcc_extract_solution_option_indices(xcc_problem* p, xcc_link* solution);

typedef void (*xcc_link_visitor)(xcc_problem* p,
                                 void* userdata,
                                 xcc_link item_index,
                                 const char* item_name);

void
xcc_extract_selected_options(xcc_problem* p,
                             void* userdata,
                             xcc_link_visitor visitor);

#ifdef __cplusplus
}

#include "xcc.hpp"
#endif

#endif
