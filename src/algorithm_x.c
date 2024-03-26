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
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <miniexact/algorithm.h>
#include <miniexact/algorithm_x.h>
#include <miniexact/ops.h>

typedef enum x_state { X1, X2, X3, X4, X5, X6, X7, X8 } x_state;

static bool
compute_next_result(miniexact_algorithm* a, miniexact_problem* p) {
  if(p->x_capacity < p->option_count) {
    p->x = realloc(p->x, sizeof(miniexact_link) * p->option_count);
    p->x_capacity = p->option_count;
    p->x_size = 0;
  }

  assert(a->choose_i);

  while(true) {
    switch(p->state) {
      case X1: {
        miniexact_link i = 0;
        do {
          i = RLINK(i);
          if(DLINK(i) == 0 && ULINK(i) == 0) {
            fprintf(stderr, "Some item never occurs in the options!\n");
            return false;
          }
        } while(RLINK(i) != 0);

        p->l = 0;
        p->state = X2;
        p->i = 0;
        break;
      }
      case X2:
        if(RLINK(0) == 0) {
          p->state = X8;
          p->x_size = p->l;
          return true;
        }
        p->state = X3;
        break;
      case X3:
        p->i = a->choose_i(a, p, 0);
        p->state = X4;
        break;
      case X4:
        COVER(p->i);
        p->x[p->l] = DLINK(p->i);
        p->state = X5;
        break;
      case X5:
        if(p->x[p->l] == p->i) {
          p->state = X7;
          break;
        } else {
          p->p = p->x[p->l] + 1;
          while(p->p != p->x[p->l]) {
            miniexact_link j = TOP(p->p);
            if(j <= 0) {
              p->p = ULINK(p->p);
            } else {
              COVER(j);
              p->p = p->p + 1;
            }
          }
        }
        p->l = p->l + 1;
        p->state = X2;
        break;
      case X6:
        p->p = p->x[p->l] - 1;
        while(p->p != p->x[p->l]) {
          miniexact_link j = TOP(p->p);
          if(j <= 0) {
            p->p = DLINK(p->p);
          } else {
            UNCOVER(j);
            p->p = p->p - 1;
          }
        }
        p->i = TOP(p->x[p->l]);
        p->x[p->l] = DLINK(p->x[p->l]);
        p->state = X5;
        break;
      case X7:
        UNCOVER(p->i);
        p->state = X8;
        break;
      case X8:
        if(p->l == 0) {
          return false;
        }
        p->l = p->l - 1;
        p->state = X6;
        break;
    }
  }

  return false;
}

void
miniexact_algorithm_x_set(miniexact_algorithm* a) {
  miniexact_algorithm_standard_functions(a);

  a->compute_next_result = &compute_next_result;
  a->choose_i = &miniexact_choose_i_mrv;
}
