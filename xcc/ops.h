#ifndef XCC_OPS_H
#define XCC_OPS_H

#define NAME(n) p->name[n]
#define LLINK(n) p->llink[n]
#define RLINK(n) p->rlink[n]
#define ULINK(n) p->ulink[n]
#define DLINK(n) p->dlink[n]
#define COLOR(n) p->color[n]
#define LEN(n) p->len[n]
#define TOP(n) p->top[n]

#include "xcc.h"

#include <assert.h>
#include <stdio.h>

inline static void
xcc_cover(xcc_problem*, xcc_link);
inline static void
xcc_uncover(xcc_problem*, xcc_link);
inline static void
xcc_hide(xcc_problem*, xcc_link);
inline static void
xcc_unhide(xcc_problem*, xcc_link);

inline static void
xcc_hide_prime(xcc_problem*, xcc_link);
inline static void
xcc_unhide_prime(xcc_problem*, xcc_link);

#define COVER(I) xcc_cover(p, I)
#define UNCOVER(I) xcc_uncover(p, I)
#define HIDE(P) xcc_hide(p, P)
#define UNHIDE(P) xcc_unhide(p, P)

#define HIDE_PRIME(P) xcc_hide_prime(p, P)
#define UNHIDE_PRIME(P) xcc_unhide_prime(p, P)

inline static void
xcc_cover(xcc_problem* p, xcc_link i_) {
  xcc_link p_ = DLINK(i_);
  while(p_ != p->i) {
    HIDE(p_);
    p_ = DLINK(p_);
  }
  xcc_link l = LLINK(i_), r = RLINK(i_);
  RLINK(l) = r;
  LLINK(r) = l;
}

inline static void
xcc_uncover(xcc_problem* p, xcc_link i) {
  xcc_link l = LLINK(i);
  xcc_link r = RLINK(i);
  RLINK(l) = i;
  LLINK(r) = i;
  xcc_link p_ = ULINK(i);
  while(p_ != i) {
    UNHIDE(p_);
    p_ = ULINK(p_);
  }
}

inline static void
xcc_hide(xcc_problem* p, xcc_link p_) {
  xcc_link q = p_ + 1;
  while(q != p_) {
    printf("Hide. p:%d, q:%d\n", p_, q);
    assert(q >= 0);
    assert(q < p->top_size);
    xcc_link x = TOP(q);
    xcc_link u = ULINK(q);
    xcc_link d = DLINK(q);
    printf("Hide. x:%d, u:%d, d:%d\n", x, u, d);

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
xcc_unhide(xcc_problem* p, xcc_link p_) {
  xcc_link q = p_ - 1;
  while(q != p_) {
    xcc_link x = TOP(q);
    xcc_link u = ULINK(q);
    xcc_link d = DLINK(q);
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
xcc_hide_prime(xcc_problem* p, xcc_link p_) {
  xcc_link q = p_ + 1;
  while(q != p_) {
    xcc_link x = TOP(q);
    xcc_link u = ULINK(q);
    xcc_link d = DLINK(q);

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
xcc_unhide_prime(xcc_problem* p, xcc_link p_) {
  xcc_link q = p_ - 1;
  while(q != p_) {
    xcc_link x = TOP(q);
    xcc_link u = ULINK(q);
    xcc_link d = DLINK(q);
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

#endif
