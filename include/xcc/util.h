#ifndef XCC_UTIL_H
#define XCC_UTIL_H

#include <stdbool.h>
#include <stdint.h>

// Small utility function to extract the sign out of an integer. Positive is
// true.
static inline bool
xcc_sign(int32_t value) {
  uint32_t temp = value;
  temp >>= 31;
  bool sign = !temp;
  return sign;
}

#endif
