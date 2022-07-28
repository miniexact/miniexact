#ifndef XCC_OPS_H
#define XCC_OPS_H

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

#include "xcc.h"

#include <stdlib.h>
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
xcc_cover_prime(xcc_problem*, xcc_link);
inline static void
xcc_uncover_prime(xcc_problem*, xcc_link);
inline static void
xcc_hide_prime(xcc_problem*, xcc_link);
inline static void
xcc_unhide_prime(xcc_problem*, xcc_link);
inline static void
xcc_commit(xcc_problem* p, xcc_link p_, xcc_link j_);
inline static void
xcc_uncommit(xcc_problem* p, xcc_link p_, xcc_link j_);
inline static void
xcc_purify(xcc_problem* p, xcc_link p_);
inline static void
xcc_unpurify(xcc_problem* p, xcc_link p_);

#define COVER(I) xcc_cover(p, I)
#define UNCOVER(I) xcc_uncover(p, I)
#define HIDE(P) xcc_hide(p, P)
#define UNHIDE(P) xcc_unhide(p, P)

#define COVER_PRIME(I) xcc_cover_prime(p, I)
#define UNCOVER_PRIME(I) xcc_uncover_prime(p, I)
#define HIDE_PRIME(P) xcc_hide_prime(p, P)
#define UNHIDE_PRIME(P) xcc_unhide_prime(p, P)

#define COMMIT(P, J) xcc_commit(p, P, J)
#define UNCOMMIT(P, J) xcc_uncommit(p, P, J)
#define PURIFY(P) xcc_purify(p, P)
#define UNPURIFY(P) xcc_unpurify(p, P)

inline static void
xcc_cover(xcc_problem* p, xcc_link i) {
  xcc_link p_ = DLINK(i);
  while(p_ != i) {
    HIDE(p_);
    p_ = DLINK(p_);
  }
  xcc_link l = LLINK(i), r = RLINK(i);
  RLINK(l) = r;
  LLINK(r) = l;
}

inline static void
xcc_cover_prime(xcc_problem* p, xcc_link i) {
  xcc_link p_ = DLINK(i);
  while(p_ != i) {
    HIDE_PRIME(p_);
    p_ = DLINK(p_);
  }
  xcc_link l = LLINK(i), r = RLINK(i);
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
xcc_uncover_prime(xcc_problem* p, xcc_link i) {
  xcc_link l = LLINK(i);
  xcc_link r = RLINK(i);
  RLINK(l) = i;
  LLINK(r) = i;
  xcc_link p_ = ULINK(i);
  while(p_ != i) {
    UNHIDE_PRIME(p_);
    p_ = ULINK(p_);
  }
}

inline static void
xcc_hide(xcc_problem* p, xcc_link p_) {
  xcc_link q = p_ + 1;
  while(q != p_) {
    assert(q >= 0);
    assert(q < p->top_size);
    xcc_link x = TOP(q);
    xcc_link u = ULINK(q);
    xcc_link d = DLINK(q);

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

inline static void
xcc_commit(xcc_problem* p, xcc_link p_, xcc_link j_) {
  if(COLOR(p_) == 0)
    COVER_PRIME(j_);
  else if(COLOR(p_) > 0)
    PURIFY(p_);
  else
    assert(false);
}

inline static void
xcc_uncommit(xcc_problem* p, xcc_link p_, xcc_link j_) {
  if(COLOR(p_) == 0)
    UNCOVER_PRIME(j_);
  else if(COLOR(p_) > 0)
    UNPURIFY(p_);
  else
    assert(false);
}

inline static void xcc_purify(xcc_problem* p, xcc_link p_) {
  xcc_link c = COLOR(p_), i = TOP(p_), q = DLINK(p_);
  while(q != i) {
    if(COLOR(q) == c)
      COLOR(q) = -1;
    else
      HIDE_PRIME(q);
    q = DLINK(q);
  }
}

inline static void xcc_unpurify(xcc_problem* p, xcc_link p_) {
  xcc_link c = COLOR(p_), i = TOP(p_), q = ULINK(p_);
  while(q != i) {
    if(COLOR(q) < 0)
      COLOR(q) = c;
    else
      UNHIDE_PRIME(q);
    q = ULINK(q);
  }
}

#ifdef __cplusplus
}
#endif

#endif
