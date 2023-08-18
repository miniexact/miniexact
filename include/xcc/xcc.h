/*
    XCCSolve - Toolset to solve exact cover problems and extensions
    Copyright (C) 2021-2023  Maximilian Heisinger

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU Affero General Public License as
    published by the Free Software Foundation, either version 3 of the
    License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Affero General Public License for more details.

    You should have received a copy of the GNU Affero General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/
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
  int print_x;
  int enumerate;
  int transform_to_libexact;
  int algorithm_select;
  char* const* input_files;
  size_t input_files_count;
  size_t current_input_file;
} xcc_config;

typedef enum xcc_algorithm_id {
  XCC_ALGORITHM_NAIVE = 1 << 1,
  XCC_ALGORITHM_MRV = 1 << 2,
  XCC_ALGORITHM_MRV_SLACKER = 1 << 3,
  XCC_ALGORITHM_X = 1 << 4,
  XCC_ALGORITHM_C = 1 << 5,
  XCC_ALGORITHM_M = 1 << 6,
  XCC_ALGORITHM_KNUTH_CNF = 1 << 7
} xcc_algorithm_id;

#define XCC_LONG_OPTIONS (1 << 20)
#define XCC_OPTION_PRINT_X (XCC_LONG_OPTIONS + 1)

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
  ARR(xcc_link, ft)
  ARR(xcc_link, slack)
  ARR(xcc_link, bound)

  // Solution
  ARR(xcc_link, x)

  int N, N_1, M, i, j, l, p, q, Z;
  int primary_item_count;
  int secondary_item_count;
  int option_count;
  int state;

  void* algorithm_userdata;
  xcc_config* cfg;
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
xcc_insert_ident_as_name(xcc_problem* p, xcc_name ident);

xcc_link
xcc_color_from_ident(xcc_problem* p, xcc_name ident);

xcc_link
xcc_color_from_ident_or_insert(xcc_problem* p, xcc_name ident);

void
xcc_print_problem_matrix(xcc_problem* p);

const char*
xcc_print_problem_matrix_in_libexact_format(xcc_problem* p);

void
xcc_print_problem_solution(xcc_problem* p);

/** @brief Extract a valid solution, consisting of the option indices
 *
 * Requires p to be in a valid solved state and solution to be a pointer to an
 * array with at least p->l elements. The real number of solutions is returned
 * by this function.
 *
 * This is basically exercise 13 combined with exercise 12.
 */
xcc_link
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
