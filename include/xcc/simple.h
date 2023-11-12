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
#ifndef XCC_SIMPLE_H
#define XCC_SIMPLE_H

// This file serves as a simple API to the XCC solving capabilities of this
// library. It is only meant for programatic use by other tools, the integrated
// command line client uses the direct XCC interface.
//
// All more complex functions are abstracted away from the compilation unit,
// also making this an easy to consume API. Functions start with xccs_ instead
// of xcc_.

#ifdef __cplusplus
extern "C" {
#endif

struct xccs;

typedef void (*xccs_solution_iterator)(struct xccs*,
                                       const char** names,
                                       const char** colors,
                                       unsigned int items_count,
                                       void* userdata);

struct xccs*
xccs_init_x();

struct xccs*
xccs_init_c();

struct xccs*
xccs_init_m();

// Define a new primary item. U and V give the slack. By default, these can both
// be 1.
void
xccs_define_primary_item(struct xccs* h, const char* name, unsigned int u, unsigned int v);

void
xccs_define_secondary_item(struct xccs* h, const char* name);

// Add an item to the current option. If the name is NULL, the option is ended.
// The color is only valid if the item is a secondary item.
void
xccs_add(struct xccs* h, const char* name, const char* color);

int
xccs_solve(struct xccs* h);

void
xccs_iterate_solution(struct xccs* h, xccs_solution_iterator it, void* userdata);

void
xccs_free(struct xccs* h);

#ifdef __cplusplus
}
#endif

#endif
