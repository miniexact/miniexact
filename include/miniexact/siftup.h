// This file implements the "siftup" algorithm from Donald Knuth,
// TAOCP Vol. 3, Page 145 (5.2.3). It tries to be as direct and
// efficient as possible, while adhering to Knuth's definitions.
//
// USAGE:
//
// Define the type you want to use for siftup array. Then, generate
// the implementation by including the file. Default is uint32_t.
//
// Requires that R is currently a heap.

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <assert.h>
#include <stddef.h>

#ifndef SIFTUP_v
#include <stdint.h>
#define SIFTUP_v uint32_t
#endif

static inline void
heap_siftup(SIFTUP_v* R, size_t N, SIFTUP_v R_) {
H1:
  size_t l = N/2 + 1;
  size_t r = N;
  R[N] = R_;

H2:
  if(l > 1) {
    l = l - 1;
    R_ = R[l];
  } else {
    R_ = R[r];
    R[r] = R[1];
    r = r - 1;
    if(r == 1) {
      R[1] = R_;
      return;
    }
  }

H3:
  size_t j = l;

H4:
  size_t i = j;
  j = 2 * j;

  assert(i == j / 2);

  if(j < r) {
    goto H5;
  } else if(j == r) {
    goto H6;
  } else {
    goto H8;
  }
H5:
  if(R[j] > R[j + 1]) {
    j = j + 1;
  }
H6:
  if(R_ <= R[j]) {
    goto H8;
  }
H7:
  R[i] = R[j];
  goto H4;
H8:
  R[i] = R_;
  goto H2;
}

#undef SIFTUP_v

#ifdef __cplusplus
}
#endif
