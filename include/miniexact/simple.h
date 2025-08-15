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
#ifndef MINIEXACT_SIMPLE_H
#define MINIEXACT_SIMPLE_H

// This file serves as a simple API to the XCC solving capabilities of this
// library. It is only meant for programatic use by other tools, the integrated
// command line client uses the direct XCC interface.
//
// All more complex functions are abstracted away from the compilation unit,
// also making this an easy to consume API. Functions start with miniexacts_
// instead of miniexact_.

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

struct miniexact_problem;
struct miniexacts;

typedef void (*miniexacts_solution_iterator)(struct miniexacts*,
                                             const char** names,
                                             const char** colors,
                                             unsigned int items_count,
                                             void* userdata);

struct miniexacts*
miniexacts_init_x();

struct miniexacts*
miniexacts_init_c();

struct miniexacts*
miniexacts_init_m();

// Define a new primary item. U and V give the slack. By default, these can both
// be 1.
int32_t
miniexacts_define_primary_item_with_slack(struct miniexacts* h,
                                          const char* name,
                                          unsigned int u,
                                          unsigned int v);

int32_t
miniexacts_define_primary_item(struct miniexacts* h, const char* name);

// Define a secondary item. It also may be colored later.
int32_t
miniexacts_define_secondary_item(struct miniexacts* h, const char* name);

// Define a new color or retrieve the ID of an existing color. Useful to avoid
// string lookups.
int32_t
miniexacts_define_color(struct miniexacts* h, const char* name);

// Add an item to the current option. If the name is NULL, the option is ended.
// The color is only valid if the item is a secondary item.
int32_t
miniexacts_add_named(struct miniexacts* h,
                     const char* name,
                     const char* color,
                     uint32_t cost);

// Add an item to the current option. If the name is 0, the option is ended.
// The color is only valid if the item is a secondary item.
int32_t
miniexacts_add(struct miniexacts* h,
               int32_t item,
               int32_t color,
               uint32_t cost);

int
miniexacts_solve(struct miniexacts* h);

bool
miniexacts_solution(struct miniexacts* h,
                    miniexacts_solution_iterator it,
                    void* userdata);

unsigned int
miniexacts_solution_length(struct miniexacts* h);

int32_t
miniexacts_extract_solution(struct miniexacts* h, int32_t* arr);

bool
miniexacts_write_to_dlx(struct miniexacts* h, const char* path);

struct miniexact_problem*
miniexacts_problem(struct miniexacts* h);

void
miniexacts_free(struct miniexacts* h);

#ifdef __cplusplus
}
#endif

#endif
