#ifndef XCC_OPS_H
#define XCC_OPS_H

#define NAME(n) p->name[n]
#define LLINK(n) p->llink[n]
#define RLINK(n) p->rlink[n]
#define ULINK(n) p->ulink[n]
#define DLINK(n) p->dlink[n]
#define LEN(n) p->len[n]
#define TOP(n) p->top[n]

#define COVER(i)                         \
  do {                                   \
    xcc_link p = DLINK(i);               \
    while(p != p->i) {                   \
      HIDE(p);                           \
      p = DLINK(p);                      \
    }                                    \
    xcc_link l = LLINK(i), r = RLINK(i); \
    RLINK(l) = r;                        \
    LLINK(r) = l;                        \
  } while(false)

#endif
