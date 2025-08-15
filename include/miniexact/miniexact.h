/*
    miniexact - Toolset to solve exact cover problems and extensions
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
#ifndef MINIEXACT_H
#define MINIEXACT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

typedef int32_t miniexact_link;
typedef miniexact_link miniexact_color;
typedef char* miniexact_name;
typedef struct miniexact_algorithm miniexact_algorithm;

#define MINIEXACT_LINK_MAX INT32_MAX

#define MINIEXACT_MAX(a, b) (((a) > (b)) ? (a) : (b))

#define ARR(TYPE, NAME) \
  TYPE* NAME;           \
  size_t NAME##_size;   \
  size_t NAME##_capacity;

#define MINIEXACT_ARR_ALLOC(TYPE, ARR)               \
  p->ARR##_capacity = 65536;                         \
  p->ARR = malloc(p->ARR##_capacity * sizeof(TYPE)); \
  p->ARR##_size = 0;

#define MINIEXACT_ARR_REALLOC(ARR)           \
  p->ARR##_capacity = p->ARR##_capacity * 4; \
  p->ARR = realloc(p->ARR, p->ARR##_capacity * sizeof(p->ARR[0]));

#define MINIEXACT_ARR_PLUSN(ARR, N)               \
  while(p->ARR##_size + N >= p->ARR##_capacity) { \
    MINIEXACT_ARR_REALLOC(ARR)                    \
  }                                               \
  p->ARR##_size += N;

#define MINIEXACT_ARR_HASN(ARR, N) \
  while(p->ARR##_capacity < N) {   \
    MINIEXACT_ARR_REALLOC(ARR)     \
  }                                \
  p->ARR##_size = MINIEXACT_MAX(N, p->ARR##_size);

#define MINIEXACT_ARR_PLUS1(ARR) MINIEXACT_ARR_PLUSN(ARR, 1)

typedef struct miniexact_config {
  int verbose;
  int print_options;
  int print_x;
  int enumerate;
  int print_dlx;
  int parse_dlx;
  int transform_to_libexact;
  int algorithm_select;
  int solutions;
  char* const* input_files;
  size_t input_files_count;
  size_t current_input_file;
} miniexact_config;

typedef enum miniexact_algorithm_id {
  MINIEXACT_ALGORITHM_NAIVE = 1 << 1,
  MINIEXACT_ALGORITHM_MRV = 1 << 2,
  MINIEXACT_ALGORITHM_MRV_SLACKER = 1 << 3,
  MINIEXACT_ALGORITHM_X = 1 << 4,
  MINIEXACT_ALGORITHM_C = 1 << 5,
  MINIEXACT_ALGORITHM_M = 1 << 6,
  MINIEXACT_ALGORITHM_KNUTH_CNF = 1 << 7,
  MINIEXACT_ALGORITHM_DOLLARS = 1 << 8,
  MINIEXACT_ALGORITHM_C_DOLLAR = 1 << 8
} miniexact_algorithm_id;

#define MINIEXACT_LONG_OPTIONS (1 << 20)
#define MINIEXACT_OPTION_PRINT_X (MINIEXACT_LONG_OPTIONS + 1)

typedef struct miniexact_problem {
  ARR(miniexact_link, llink)
  ARR(miniexact_link, rlink)
  ARR(miniexact_link, ulink)
  ARR(miniexact_link, dlink)
#ifndef SWIG
  union {
    struct {
#endif
      ARR(miniexact_link, top)
#ifndef SWIG
    };
    struct {
      ARR(miniexact_link, len)
    };
  };
#endif

  ARR(miniexact_name, name)
  ARR(miniexact_name, color_name)
  ARR(miniexact_color, color)
  ARR(miniexact_link, ft)
  ARR(miniexact_link, slack)
  ARR(miniexact_link, bound)
  ARR(int32_t, cost)
  ARR(int32_t, best)
  ARR(int32_t, tho)
  ARR(int32_t, th)

  // Solution
  ARR(miniexact_link, x)

  int N, N_1, M, i, j, l, p, q, Z, K;
  int primary_item_count;
  int secondary_item_count;
  int option_count;
  int state;
  int longest_option;
  int32_t max_option_cost;

  void* algorithm_userdata;
  miniexact_config* cfg;
} miniexact_problem;

#undef ARR

int
miniexact_search_for_name(const char* needle,
                          const miniexact_name* names,
                          size_t names_size);

bool
miniexact_has_item(miniexact_link needle, miniexact_link* list, size_t len);

miniexact_problem*
miniexact_problem_allocate(void);

void
miniexact_problem_free_inner(miniexact_problem* p, miniexact_algorithm* a);

void
miniexact_problem_free(miniexact_problem* p, miniexact_algorithm* a);

miniexact_link
miniexact_item_from_ident(miniexact_problem* p, const char* ident);

miniexact_link
miniexact_insert_ident_as_name(miniexact_problem* p, const char* ident);

void
miniexact_append_NULL_to_name(miniexact_problem* p);

miniexact_link
miniexact_color_from_ident(miniexact_problem* p, const char* ident);

miniexact_link
miniexact_color_from_ident_or_insert(miniexact_problem* p, const char* ident);

void
miniexact_print_problem_matrix(miniexact_problem* p);

const char*
miniexact_print_problem_matrix_in_libexact_format(miniexact_problem* p);

void
miniexact_write_problem_to_dlx(miniexact_problem* p, FILE* o);

void
miniexact_print_problem_solution(miniexact_problem* p);

/** @brief Extract a valid solution, consisting of the option indices
 *
 * Requires p to be in a valid solved state and solution to be a pointer to an
 * array with at least p->l elements. The real number of solutions is returned
 * by this function.
 *
 * This is basically exercise 13 combined with exercise 12.
 */
miniexact_link
miniexact_extract_solution_option_indices(miniexact_problem* p,
                                          miniexact_link* solution);

typedef void (*miniexact_link_visitor)(miniexact_problem* p,
                                       void* userdata,
                                       miniexact_link item_index,
                                       const char* item_name);

#ifdef __cplusplus
}

#include "miniexact.hpp"
#endif

#endif
