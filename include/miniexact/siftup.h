// This file implements the "siftup" algorithm from Donald Knuth,
// TAOCP Vol. 3, Page 145 (5.2.3). It tries to be as direct and
// efficient as possible, while adhering to Knuth's definitions.
//
// USAGE:
//
// Define the type you want to use for siftup array. Then, generate
// the implementation by including the file. Default is int32_t.
//
// Requires that R is currently a heap.

#ifdef __cplusplus
extern "C" {
#endif

#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#ifndef SIFTUP_v
#include <stdint.h>
#define SIFTUP_v int32_t
#endif

static inline int
heap_siftup_compare(const void* l, const void* r) {
  return *((const SIFTUP_v*)l) < *((const SIFTUP_v*)r);
}

static inline void
heap_siftup(SIFTUP_v* R, size_t N, SIFTUP_v R_) {
  // TODO: Modify Donald Knuth's Heapsort into a Max-Heap variant to
  // more efficiently insert elements.
  //
  // We want to directly exploit the heap property to efficiently
  // insert R_ into the binary tree.
  R[0] = R_;
  qsort(R, N, sizeof(SIFTUP_v), &heap_siftup_compare);
}

#undef SIFTUP_v

#ifdef __cplusplus
}
#endif
