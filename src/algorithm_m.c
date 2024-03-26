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
#include <miniexact/algorithm_m.h>
#include <miniexact/ops.h>

typedef enum m_state { M1, M2, M3, M4, M5, M6, M7, M8, M9 } m_state;

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
      case M1: {
        miniexact_link i = 0;
        do {
          i = RLINK(i);
          if(DLINK(i) == 0 && ULINK(i) == 0) {
            fprintf(stderr, "Some item never occurs in the options!\n");
            return false;
          }
        } while(RLINK(i) != 0);

        p->l = 0;
        p->state = M2;
        p->i = 0;
        break;
      }
      case M2:
        if(RLINK(0) == 0) {
          p->state = M9;
          p->x_size = p->l;
          return true;
        }
        p->state = M3;
        break;
      case M3:
        p->i = a->choose_i(a, p, 0);
        assert(p->i <= p->primary_item_count);
        if(THETA(p->i) == 0) {
          p->state = M9;
        } else {
          p->state = M4;
        }
        break;
      case M4:
        p->x[p->l] = DLINK(p->i);
        BOUND(p->i) = BOUND(p->i) - 1;
        if(BOUND(p->i) == 0)
          COVER_PRIME(p->i);
        if(BOUND(p->i) != 0 || SLACK(p->i) != 0)
          FT(p->l) = p->x[p->l];
        p->state = M5;
        break;
      case M5:
        if(BOUND(p->i) == 0 && BOUND(p->i) == SLACK(p->i)) {
          if(p->x[p->l] != p->i) {
            p->state = M6;
          } else {
            p->state = M8;
          }
          break;
        }
        if(LEN(p->i) <= BOUND(p->i) - SLACK(p->i)) {
          p->state = M8;// List i is too short
          break;
        }
        if(p->x[p->l] != p->i) {
          if(BOUND(p->i) == 0)
            TWEAK_PRIME(p->x[p->l], p->i);
          else
            TWEAK(p->x[p->l], p->i);
        } else if(BOUND(p->i) != 0) {
          p->p = LLINK(p->i);
          p->q = RLINK(p->i);
          RLINK(p->p) = p->q;
          LLINK(p->q) = p->p;
        }
        p->state = M6;
        break;
      case M6:
        if(p->x[p->l] != p->i) {
          p->p = p->x[p->l] + 1;
          assert(p->p < p->top_size);
          while(p->x[p->l] != p->p) {
            miniexact_link j = TOP(p->p);
            if(j <= 0) {
              p->p = ULINK(p->p);
            } else if(j <= p->N_1) {
              BOUND(j) = BOUND(j) - 1;
              p->p = p->p + 1;
              if(BOUND(j) == 0) {
                COVER_PRIME(j);
              }
            } else {
              COMMIT(p->p, j);
              p->p = p->p + 1;
            }
          }
        }
        p->l = p->l + 1;
        p->state = M2;
        break;
      case M7:
        p->p = p->x[p->l] - 1;
        while(p->x[p->l] != p->p) {
          miniexact_link j = TOP(p->p);
          if(j <= 0) {
            p->p = DLINK(p->p);
          } else if(j <= p->N_1) {
            BOUND(j) = BOUND(j) + 1;
            p->p = p->p - 1;
            if(BOUND(j) == 1) {
              UNCOVER_PRIME(j);
            }
          } else {
            UNCOMMIT(p->p, j);
            p->p = p->p - 1;
          }
        }
        p->x[p->l] = DLINK(p->x[p->l]);
        p->state = M5;
        break;
      case M8:
        if(BOUND(p->i) == 0 && BOUND(p->i) == SLACK(p->i)) {
          UNCOVER_PRIME(p->i);
        } else if(BOUND(p->i) == 0) {
          UNTWEAK_PRIME(p->l);
        } else {
          UNTWEAK(p->l);
        }
        BOUND(p->i) = BOUND(p->i) + 1;
        p->state = M9;
        break;
      case M9:
        if(p->l == 0) {
          return false;
        }
        p->l = p->l - 1;
        if(p->x[p->l] <= p->N) {
          p->i = p->x[p->l];
          p->p = LLINK(p->i);
          p->q = RLINK(p->i);
          RLINK(p->p) = p->i;
          LLINK(p->q) = p->i;
          p->state = M8;
          break;
        } else {
          p->i = TOP(p->x[p->l]);
          p->state = M7;
          break;
        }
    }
  }

  return false;
}

void
miniexact_algorithm_m_set(miniexact_algorithm* a) {
  miniexact_algorithm_standard_functions(a);

  a->compute_next_result = &compute_next_result;
  // Override the default MRV heuristic with the slacker MRV heuristic.
  a->choose_i = &miniexact_choose_i_mrv_slacker;
}
