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
#ifndef MINIEXACT_UTIL_H
#define MINIEXACT_UTIL_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// Small utility function to extract the sign out of an integer. Positive is
// true.
static inline bool
miniexact_sign(int32_t value) {
  uint32_t temp = value;
  temp >>= 31;
  bool sign = !temp;
  return sign;
}

struct miniexact_algorithm;
struct miniexact_problem;
struct miniexact_config;

// Utility function to solve the given problem and print solutions. Both used in
// the web version and the CLI version of miniexactsolve.
int
miniexact_solve_problem_and_print_solutions(struct miniexact_algorithm* a,
                                      struct miniexact_problem* p,
                                      struct miniexact_config* cfg);

// Utility function to solve a given problem and return if there are solutions.
int
miniexact_solve_problem(struct miniexact_algorithm* a, struct miniexact_problem* p);

typedef void (*miniexact_option_str_iterator)(struct miniexact_problem*,
                                        const char** names,
                                        const char** colors,
                                        size_t items_count,
                                        void* userdata);

// Utility function to iterate over the options that solve a problem. Represents
// items and colors as strings.
void
miniexact_iterate_solution_options_str(struct miniexact_problem* p,
                                 miniexact_option_str_iterator it,
                                 void* userdata);

#ifdef __cplusplus
}
#endif

#endif
