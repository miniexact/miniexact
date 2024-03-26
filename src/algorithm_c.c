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
#include <miniexact/algorithm_c.h>
#include <miniexact/ops.h>

typedef enum c_state { C1, C2, C3, C4, C5, C6, C7, C8 } c_state;

static bool
compute_next_result(miniexact_algorithm* a, miniexact_problem* p) {
  if(p->x_capacity < p->option_count) {
    p->x = realloc(p->x, sizeof(miniexact_link) * p->option_count);
    p->x_capacity = p->option_count;
    p->x_size = 0;
  }

  assert(a->choose_i);

  while(true) {
    // State print:
    // printf("State: %d i:%d l:%d x[0]:%d\n", p->state, p->i, p->l, p->x[0]);
    switch(p->state) {
      case C1: {
        miniexact_link i = 0;
        do {
          i = RLINK(i);
          if(DLINK(i) == 0 && ULINK(i) == 0) {
            fprintf(stderr, "Some item never occurs in the options!\n");
            return false;
          }
        } while(RLINK(i) != 0);

        p->l = 0;
        p->state = C2;
        p->i = 0;
        break;
      }
      case C2:
        if(RLINK(0) == 0) {
          p->state = C8;
          p->x_size = p->l;
          return true;
        }
        p->state = C3;
        break;
      case C3:
        p->i = a->choose_i(a, p, 0);
        p->state = C4;
        break;
      case C4:
        COVER_PRIME(p->i);
        p->x[p->l] = DLINK(p->i);
        p->state = C5;
        break;
      case C5:
        if(p->x[p->l] == p->i) {
          p->state = C7;
          break;
        }
        p->p = p->x[p->l] + 1;
        while(p->p != p->x[p->l]) {
          miniexact_link j = TOP(p->p);
          if(j <= 0) {
            p->p = ULINK(p->p);
          } else {
            COMMIT(p->p, j);
            p->p = p->p + 1;
          }
        }
        p->l = p->l + 1;
        p->state = C2;
        break;
      case C6:
        p->p = p->x[p->l] - 1;
        while(p->p != p->x[p->l]) {
          miniexact_link j = TOP(p->p);
          if(j <= 0) {
            p->p = DLINK(p->p);
          } else {
            UNCOMMIT(p->p, j);
            p->p = p->p - 1;
          }
        }
        p->i = TOP(p->x[p->l]);
        p->x[p->l] = DLINK(p->x[p->l]);
        p->state = C5;
        break;
      case C7:
        UNCOVER_PRIME(p->i);
        p->state = C8;
        break;
      case C8:
        if(p->l == 0) {
          return false;
        }
        p->l = p->l - 1;
        p->state = C6;
        break;
    }
  }

  return false;
}

void
miniexact_algorithm_c_set(miniexact_algorithm* a) {
  miniexact_algorithm_standard_functions(a);

  a->compute_next_result = &compute_next_result;
  a->choose_i = &miniexact_choose_i_mrv;
}
