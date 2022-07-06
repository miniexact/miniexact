#ifndef XCC_OPS_H
#define XCC_OPS_H

#define NAME(n) p->name[n]
#define LLINK(n) p->llink[n]
#define RLINK(n) p->rlink[n]
#define ULINK(n) p->ulink[n]
#define DLINK(n) p->dlink[n]
#define LEN(n) p->len[n]
#define TOP(n) p->top[n]

#define COVER(i_)                          \
  do {                                     \
    xcc_link p_ = DLINK(i_);               \
    while(p_ != p->i) {                    \
      HIDE(p_);                            \
      p_ = DLINK(p_);                      \
    }                                      \
    xcc_link l = LLINK(i_), r = RLINK(i_); \
    RLINK(l) = r;                          \
    LLINK(r) = l;                          \
  } while(false)

#define UNCOVER(i)          \
  do {                      \
    xcc_link l = LLINK(i);  \
    xcc_link r = RLINK(i);  \
    RLINK(l) = i;           \
    LLINK(r) = i;           \
    xcc_link p_ = ULINK(i); \
    while(p_ != i) {        \
      UNHIDE(p_);           \
      p_ = ULINK(p_);       \
    }                       \
  } while(false)

#define HIDE(p_)                    \
  do {                              \
    xcc_link q = p_ + 1;            \
    while(q != p_) {                \
      xcc_link x = TOP(q);          \
      xcc_link u = ULINK(q);        \
      xcc_link d = DLINK(q);        \
                                    \
      if(x <= 0) {                  \
        q = u; /* q was a spacer */ \
      } else {                      \
        DLINK(u) = d;               \
        ULINK(d) = u;               \
        LEN(x) = LEN(x) - 1;        \
        q = q + 1;                  \
      }                             \
    }                               \
  } while(false)

#define UNHIDE(p_)                  \
  do {                              \
    xcc_link q = p_ - 1;            \
    while(q != p_) {                \
      xcc_link x = TOP(q);          \
      xcc_link u = ULINK(q);        \
      xcc_link d = DLINK(q);        \
      if(x <= 0) {                  \
        q = d; /* q was a spacer */ \
      } else {                      \
        DLINK(u) = q;               \
        ULINK(d) = q;               \
        LEN(x) = LEN(x) + 1;        \
        q = q - 1;                  \
      }                             \
    }                               \
  } while(false)

#define HIDE_PRIME(p_)              \
  do {                              \
    xcc_link q = p_ + 1;            \
    while(q != p_) {                \
      xcc_link x = TOP(q);          \
      xcc_link u = ULINK(q);        \
      xcc_link d = DLINK(q);        \
                                    \
      if(x <= 0) {                  \
        q = u; /* q was a spacer */ \
      } else if(COLOR(q) < 0) {     \
        q = q + 1;                  \
      } else {                      \
        DLINK(u) = d;               \
        ULINK(d) = u;               \
        LEN(x) = LEN(x) - 1;        \
        q = q + 1;                  \
      }                             \
    }                               \
  } while(false)

#define UNHIDE_PRIME(p_)            \
  do {                              \
    xcc_link q = p_ - 1;            \
    while(q != p_) {                \
      xcc_link x = TOP(q);          \
      xcc_link u = ULINK(q);        \
      xcc_link d = DLINK(q);        \
      if(x <= 0) {                  \
        q = d; /* q was a spacer */ \
      } else if(COLOR(q) < 0) {     \
        q = q - 1;                  \
      } else {                      \
        DLINK(u) = q;               \
        ULINK(d) = q;               \
        LEN(x) = LEN(x) + 1;        \
        q = q - 1;                  \
      }                             \
    }                               \
  } while(false)

#endif
