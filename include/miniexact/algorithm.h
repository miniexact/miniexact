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
#ifndef MINIEXACT_ALGORITHM_H
#define MINIEXACT_ALGORITHM_H

#include "miniexact.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct miniexact_algorithm miniexact_algorithm;

typedef const char* (*miniexact_define_primary_item)(miniexact_algorithm* a,
                                                     miniexact_problem* p,
                                                     miniexact_link n);
typedef const char* (*miniexact_define_primary_item_with_range)(
  miniexact_algorithm* a,
  miniexact_problem* p,
  miniexact_link n,
  miniexact_link slack,
  miniexact_link bound);

typedef const char* (*miniexact_define_secondary_item)(miniexact_algorithm* a,
                                                       miniexact_problem* p,
                                                       miniexact_link n);

typedef const char* (*miniexact_add_item)(miniexact_algorithm* a,
                                          miniexact_problem* p,
                                          miniexact_link l);
typedef const char* (*miniexact_add_item_with_color)(miniexact_algorithm* a,
                                                     miniexact_problem* p,
                                                     miniexact_link l,
                                                     miniexact_color color);
typedef const char* (*miniexact_end_option)(miniexact_algorithm* a,
                                            miniexact_problem* p,
                                            int32_t cost);
typedef const char* (*miniexact_prepare_options)(miniexact_algorithm* a,
                                                 miniexact_problem* p);
typedef const char* (*miniexact_end_options)(miniexact_algorithm* a,
                                             miniexact_problem* p);

typedef const char* (*miniexact_init_problem)(miniexact_algorithm* a,
                                              miniexact_problem* p);

typedef bool (*miniexact_compute_next_result)(miniexact_algorithm* a,
                                              miniexact_problem* p);

typedef miniexact_link (*miniexact_choose_i)(miniexact_algorithm* a,
                                             miniexact_problem* p,
                                             int32_t T);

typedef void (*miniexact_userdata_free)(miniexact_algorithm* a,
                                        miniexact_problem* p);

miniexact_link
miniexact_choose_i_naively(miniexact_algorithm* a,
                           miniexact_problem* p,
                           int32_t t);

miniexact_link
miniexact_choose_i_naively_cost(miniexact_algorithm* a,
                                miniexact_problem* p,
                                int32_t t);

miniexact_link
miniexact_choose_i_mrv(miniexact_algorithm* a, miniexact_problem* p, int32_t t);

miniexact_link
miniexact_choose_i_mrv_cost(miniexact_algorithm* a,
                            miniexact_problem* p,
                            int32_t t);

miniexact_link
miniexact_choose_i_mrv_slacker(miniexact_algorithm* a,
                               miniexact_problem* p,
                               int32_t t);

typedef struct miniexact_algorithm {
  miniexact_define_primary_item define_primary_item;
  miniexact_define_primary_item_with_range define_primary_item_with_range;
  miniexact_define_secondary_item define_secondary_item;
  miniexact_end_option end_option;
  miniexact_prepare_options prepare_options;
  miniexact_add_item add_item;
  miniexact_add_item_with_color add_item_with_color;
  miniexact_end_options end_options;

  miniexact_init_problem init_problem;

  miniexact_compute_next_result compute_next_result;

  miniexact_choose_i choose_i;

  miniexact_userdata_free free_userdata;
} miniexact_algorithm;

void
miniexact_algorithm_standard_functions(miniexact_algorithm* algorithm);

const char*
miniexact_default_init_problem(miniexact_algorithm* a, miniexact_problem* p);

bool
miniexact_algorithm_from_select(int algorithm_select,
                                miniexact_algorithm* algorithm);

miniexact_algorithm*
miniexact_algorithm_allocate(void);
miniexact_algorithm*
miniexact_algorithm_x_allocate(void);
miniexact_algorithm*
miniexact_algorithm_c_allocate(void);
miniexact_algorithm*
miniexact_algorithm_m_allocate(void);

#ifdef __cplusplus
}
#endif

#endif
