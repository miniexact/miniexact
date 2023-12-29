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
#ifndef MINIEXACT_SAT_SOLVER
#define MINIEXACT_SAT_SOLVER

#include <stdio.h>
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct miniexact_sat_solver {
  int infd[2];
  int outfd[2];
  FILE* infd_handle;
  FILE* outfd_handle;
  pid_t pid;
  unsigned int variables, clauses;
  char* assignments;
} miniexact_sat_solver;

void
miniexact_sat_solver_init(miniexact_sat_solver* solver,
                    unsigned int variables,
                    unsigned int clauses,
                    char* binary,
                    char* const argv[],
                    char* envp[]);

void
miniexact_sat_solver_find_and_init(miniexact_sat_solver* solver,
                             unsigned int variables,
                             unsigned int clauses);

void
miniexact_sat_solver_destroy(miniexact_sat_solver* solver);

void
miniexact_sat_solver_add(miniexact_sat_solver* solver, int l);
void
miniexact_sat_solver_unit(miniexact_sat_solver* solver, int l);
void
miniexact_sat_solver_binary(miniexact_sat_solver* solver, int a, int b);
void
miniexact_sat_solver_ternary(miniexact_sat_solver* solver, int a, int b, int c);

int
miniexact_sat_solver_solve(miniexact_sat_solver* solver);

#ifdef __cplusplus
}
#endif

#endif
