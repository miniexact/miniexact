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
#ifndef MINIEXACT_OPS_H
#define MINIEXACT_OPS_H

#ifdef __cplusplus
extern "C" {
#endif

#define NAME(n) p->name[n]
#define LLINK(n) p->llink[n]
#define RLINK(n) p->rlink[n]
#define ULINK(n) p->ulink[n]
#define DLINK(n) p->dlink[n]
#define COLOR(n) p->color[n]
#define LEN(n) p->len[n]
#define TOP(n) p->top[n]
#define FT(l) p->ft[l]
#define SLACK(l) p->slack[l]
#define BOUND(l) p->bound[l]
#define BEST(l) p->best[l]
#define COST(l) p->cost[l]
#define THO(l) p->tho[l]
#define TH(l) p->th[l]

#include "miniexact.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

inline static void
miniexact_cover(miniexact_problem*, miniexact_link);
inline static void
miniexact_uncover(miniexact_problem*, miniexact_link);
inline static void
miniexact_hide(miniexact_problem*, miniexact_link);
inline static void
miniexact_unhide(miniexact_problem*, miniexact_link);

inline static void
miniexact_cover_prime(miniexact_problem*, miniexact_link);
inline static void
miniexact_uncover_prime(miniexact_problem*, miniexact_link);
inline static void
miniexact_hide_prime(miniexact_problem*, miniexact_link);
inline static void
miniexact_unhide_prime(miniexact_problem*, miniexact_link);
inline static void
miniexact_commit(miniexact_problem* p, miniexact_link p_, miniexact_link j_);
inline static void
miniexact_uncommit(miniexact_problem* p, miniexact_link p_, miniexact_link j_);
inline static void
miniexact_commit_threshold(miniexact_problem* p,
                           miniexact_link p_,
                           miniexact_link j_,
                           int32_t t);
inline static void
miniexact_uncommit_threshold(miniexact_problem* p,
                             miniexact_link p_,
                             miniexact_link j_,
                             int32_t t);
inline static void
miniexact_purify(miniexact_problem* p, miniexact_link p_);
inline static void
miniexact_unpurify(miniexact_problem* p, miniexact_link p_);
inline static void
miniexact_purify_threshold(miniexact_problem* p, miniexact_link p_, int32_t t);
inline static void
miniexact_unpurify_threshold(miniexact_problem* p,
                             miniexact_link p_,
                             int32_t t);

inline static void
miniexact_tweak(miniexact_problem* p, miniexact_link x_, miniexact_link p_);
inline static void
miniexact_untweak(miniexact_problem* p, miniexact_link l);
inline static void
miniexact_tweak_prime(miniexact_problem* p,
                      miniexact_link x_,
                      miniexact_link p_);
inline static void
miniexact_untweak_prime(miniexact_problem* p, miniexact_link l);
inline static void
miniexact_unhide_prime_dollar(miniexact_problem* p, miniexact_link p_);

#define COVER(I) miniexact_cover(p, I)
#define UNCOVER(I) miniexact_uncover(p, I)
#define HIDE(P) miniexact_hide(p, P)
#define UNHIDE(P) miniexact_unhide(p, P)

#define COVER_PRIME(I) miniexact_cover_prime(p, I)
#define UNCOVER_PRIME(I) miniexact_uncover_prime(p, I)
#define HIDE_PRIME(P) miniexact_hide_prime(p, P)
#define UNHIDE_PRIME(P) miniexact_unhide_prime(p, P)

#define COMMIT(P, J) miniexact_commit(p, P, J)
#define UNCOMMIT(P, J) miniexact_uncommit(p, P, J)
#define PURIFY(P) miniexact_purify(p, P)
#define UNPURIFY(P) miniexact_unpurify(p, P)

#define TWEAK(X, P) miniexact_tweak(p, X, P)
#define UNTWEAK(L) miniexact_untweak(p, L)
#define TWEAK_PRIME(X, P) miniexact_tweak_prime(p, X, P)
#define UNTWEAK_PRIME(L) miniexact_untweak_prime(p, L)

#define COVER_PRIME_THRESHOLD(I, T) miniexact_cover_prime_threshold(p, I, T)
#define UNCOVER_PRIME_THRESHOLD(I, T) miniexact_uncover_prime_threshold(p, I, T)
#define PURIFY_THRESHOLD(P, T) miniexact_purify_threshold(p, P, T)
#define UNPURIFY_THRESHOLD(P, T) miniexact_unpurify_threshold(p, P, T)
#define COMMIT_THRESHOLD(P, J, T) miniexact_commit_threshold(p, P, J, T)
#define UNCOMMIT_THRESHOLD(P, J, T) miniexact_uncommit_threshold(p, P, J, T)

#define UNHIDE_PRIME_DOLLAR(P) miniexact_unhide_prime_dollar(p, P)

#define PART_SOL_COST() miniexact_partial_solution_cost(p)

// Taken from https://stackoverflow.com/a/3437484
#define MAX(a,b) (((a)>(b))?(a):(b))

// Defined in Fascicle 5, page 271 (Answer to exercise 166)
#define MONUS(X, Y) MAX(X - Y, 0)
#define THETA(P) MONUS(LEN(P) + 1, MONUS(BOUND(P), SLACK(P)))

inline static void
miniexact_cover(miniexact_problem* p, miniexact_link i) {
  miniexact_link p_ = DLINK(i);
  while(p_ != i) {
    HIDE(p_);
    p_ = DLINK(p_);
  }
  miniexact_link l = LLINK(i), r = RLINK(i);
  RLINK(l) = r;
  LLINK(r) = l;
}

inline static void
miniexact_cover_prime(miniexact_problem* p, miniexact_link i) {
  miniexact_link p_ = DLINK(i);
  while(p_ != i) {
    HIDE_PRIME(p_);
    p_ = DLINK(p_);
  }
  miniexact_link l = LLINK(i), r = RLINK(i);
  RLINK(l) = r;
  LLINK(r) = l;
}

inline static void
miniexact_cover_prime_threshold(miniexact_problem* p,
                                miniexact_link i,
                                int32_t t) {
  miniexact_link p_ = DLINK(i);
  while(p_ != i && COST(p_) < t) {
    HIDE_PRIME(p_);
    p_ = DLINK(p_);
  }
  miniexact_link l = LLINK(i), r = RLINK(i);
  RLINK(l) = r;
  LLINK(r) = l;
}

inline static void
miniexact_uncover(miniexact_problem* p, miniexact_link i) {
  miniexact_link l = LLINK(i);
  miniexact_link r = RLINK(i);
  RLINK(l) = i;
  LLINK(r) = i;
  miniexact_link p_ = ULINK(i);
  while(p_ != i) {
    UNHIDE(p_);
    p_ = ULINK(p_);
  }
}

inline static void
miniexact_uncover_prime(miniexact_problem* p, miniexact_link i) {
  miniexact_link l = LLINK(i);
  miniexact_link r = RLINK(i);
  RLINK(l) = i;
  LLINK(r) = i;
  miniexact_link p_ = ULINK(i);
  while(p_ != i) {
    UNHIDE_PRIME(p_);
    p_ = ULINK(p_);
  }
}

inline static void
miniexact_uncover_prime_threshold(miniexact_problem* p,
                                  miniexact_link i,
                                  int32_t t) {
  miniexact_link l = LLINK(i);
  miniexact_link r = RLINK(i);
  RLINK(l) = i;
  LLINK(r) = i;
  miniexact_link p_ = ULINK(i);
  while(p_ != i && COST(p_) < t) {
    UNHIDE_PRIME_DOLLAR(p_);
    p_ = DLINK(p_);
  }
}

inline static void
miniexact_hide(miniexact_problem* p, miniexact_link p_) {
  miniexact_link q = p_ + 1;
  while(q != p_) {
    assert(q >= 0);
    assert(((size_t)q) < p->top_size);
    miniexact_link x = TOP(q);
    miniexact_link u = ULINK(q);
    miniexact_link d = DLINK(q);

    assert(x != 0);

    if(x <= 0) {
      q = u; /* q was a spacer */
    } else {
      DLINK(u) = d;
      ULINK(d) = u;
      LEN(x) = LEN(x) - 1;
      q = q + 1;
    }
  }
}

inline static void
miniexact_unhide(miniexact_problem* p, miniexact_link p_) {
  miniexact_link q = p_ - 1;
  while(q != p_) {
    miniexact_link x = TOP(q);
    miniexact_link u = ULINK(q);
    miniexact_link d = DLINK(q);
    if(x <= 0) {
      q = d; /* q was a spacer */
    } else {
      DLINK(u) = q;
      ULINK(d) = q;
      LEN(x) = LEN(x) + 1;
      q = q - 1;
    }
  }
}

inline static void
miniexact_hide_prime(miniexact_problem* p, miniexact_link p_) {
  miniexact_link q = p_ + 1;
  while(q != p_) {
    miniexact_link x = TOP(q);
    miniexact_link u = ULINK(q);
    miniexact_link d = DLINK(q);

    if(x <= 0) {
      q = u; /* q was a spacer */
    } else if(COLOR(q) < 0) {
      q = q + 1;
    } else {
      DLINK(u) = d;
      ULINK(d) = u;
      LEN(x) = LEN(x) - 1;
      q = q + 1;
    }
  }
}

inline static void
miniexact_unhide_prime(miniexact_problem* p, miniexact_link p_) {
  miniexact_link q = p_ - 1;
  while(q != p_) {
    miniexact_link x = TOP(q);
    miniexact_link u = ULINK(q);
    miniexact_link d = DLINK(q);
    if(x <= 0) {
      q = d; /* q was a spacer */
    } else if(COLOR(q) < 0) {
      q = q - 1;
    } else {
      DLINK(u) = q;
      ULINK(d) = q;
      LEN(x) = LEN(x) + 1;
      q = q - 1;
    }
  }
}

inline static void
miniexact_unhide_prime_dollar(miniexact_problem* p, miniexact_link p_) {
  miniexact_link q = p_ - 1;
  while(q != p_) {
    miniexact_link x = TOP(q);
    miniexact_link u = ULINK(q);
    miniexact_link d = DLINK(q);
    if(x <= 0) {
      q = d; /* q was a spacer */
    } else if(COLOR(q) < 0) {
      q = q + 1;
    } else {
      DLINK(u) = q;
      ULINK(d) = q;
      LEN(x) = LEN(x) + 1;
      q = q - 1;
    }
  }
}

inline static void
miniexact_commit(miniexact_problem* p, miniexact_link p_, miniexact_link j_) {
  if(COLOR(p_) == 0)
    COVER_PRIME(j_);
  else if(COLOR(p_) > 0)
    PURIFY(p_);
}

inline static void
miniexact_uncommit(miniexact_problem* p, miniexact_link p_, miniexact_link j_) {
  if(COLOR(p_) == 0)
    UNCOVER_PRIME(j_);
  else if(COLOR(p_) > 0)
    UNPURIFY(p_);
}

inline static void
miniexact_commit_threshold(miniexact_problem* p,
                           miniexact_link p_,
                           miniexact_link j_,
                           int32_t t) {
  if(COLOR(p_) == 0)
    COVER_PRIME_THRESHOLD(j_, t);
  else if(COLOR(p_) > 0)
    PURIFY_THRESHOLD(p_, t);
}

inline static void
miniexact_uncommit_threshold(miniexact_problem* p,
                             miniexact_link p_,
                             miniexact_link j_,
                             int32_t t) {
  if(COLOR(p_) == 0)
    UNCOVER_PRIME_THRESHOLD(j_, t);
  else if(COLOR(p_) > 0)
    UNPURIFY_THRESHOLD(p_, t);
}

inline static void
miniexact_purify(miniexact_problem* p, miniexact_link p_) {
  miniexact_link c = COLOR(p_);
  miniexact_link i = TOP(p_);
  // Inserted according to err4f5 (Errata)
  COLOR(i) = c;
  miniexact_link q = DLINK(i);
  while(q != i) {
    if(COLOR(q) == c)
      COLOR(q) = -1;
    else
      HIDE_PRIME(q);
    q = DLINK(q);
  }
}

inline static void
miniexact_purify_threshold(miniexact_problem* p, miniexact_link p_, int32_t t) {
  miniexact_link c = COLOR(p_);
  miniexact_link i = TOP(p_);
  // Inserted according to err4f5 (Errata)
  COLOR(i) = c;
  miniexact_link q = DLINK(i);
  while(q != i && COST(q) < t) {
    if(COLOR(q) == c)
      COLOR(q) = -1;
    else
      HIDE_PRIME(q);
    q = DLINK(q);
  }
}

inline static void
miniexact_unpurify(miniexact_problem* p, miniexact_link p_) {
  miniexact_link c = COLOR(p_), i = TOP(p_), q = ULINK(i);
  while(q != i) {
    if(COLOR(q) < 0)
      COLOR(q) = c;
    else
      UNHIDE_PRIME(q);
    q = ULINK(q);
  }
}

inline static void
miniexact_unpurify_threshold(miniexact_problem* p,
                             miniexact_link p_,
                             int32_t t) {
  miniexact_link c = COLOR(p_), i = TOP(p_), q = ULINK(i);
  while(q != i && COST(q) < t) {
    if(COLOR(q) < 0)
      COLOR(q) = c;
    else
      UNHIDE_PRIME_DOLLAR(q);
    q = DLINK(q);
  }
}

inline static void
miniexact_tweak(miniexact_problem* p, miniexact_link x, miniexact_link p_) {
  assert(x == DLINK(p_));
  assert(p_ == ULINK(x));
  HIDE_PRIME(x);
  miniexact_link d = DLINK(x);
  DLINK(p_) = d;
  ULINK(d) = p_;
  LEN(p_) = LEN(p_) - 1;
}

inline static void
miniexact_untweak(miniexact_problem* p, miniexact_link l) {
  miniexact_link a = FT(l);
  miniexact_link p_ = (a <= p->N ? a : TOP(a));
  miniexact_link x = a, y = p_;
  miniexact_link z = DLINK(p_);
  DLINK(p_) = x;
  miniexact_link k = 0;
  while(x != z) {
    ULINK(x) = y;
    k = k + 1;
    UNHIDE_PRIME(x);
    y = x;
    x = DLINK(x);
  }
  ULINK(z) = y;
  LEN(p_) = LEN(p_) + k;
}

inline static void
miniexact_tweak_prime(miniexact_problem* p,
                      miniexact_link x,
                      miniexact_link p_) {
  assert(x == DLINK(p_));
  assert(p_ == ULINK(x));
  miniexact_link d = DLINK(x);
  DLINK(p_) = d;
  ULINK(d) = p_;
  LEN(p_) = LEN(p_) - 1;
}

inline static void
miniexact_untweak_prime(miniexact_problem* p, miniexact_link l) {
  miniexact_link a = FT(l);
  miniexact_link p_ = (a <= p->N ? a : TOP(a));
  miniexact_link x = a, y = p_;
  miniexact_link z = DLINK(p_);
  DLINK(p_) = x;
  miniexact_link k = 0;
  while(x != z) {
    ULINK(x) = y;
    k = k + 1;
    y = x;
    x = DLINK(x);
  }
  ULINK(z) = y;
  LEN(p_) = LEN(p_) + k;
  UNCOVER_PRIME(p_);
}

inline static int32_t
miniexact_partial_solution_cost(miniexact_problem* p) {
  int32_t cost = 0;
  for(miniexact_link i = 0; i < p->l; ++i) {
    cost += COST(p->x[i]);
  }
  return cost;
}

#ifdef __cplusplus
}
#endif

#endif
