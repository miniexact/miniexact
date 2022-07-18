#ifndef XCC_LOG_H
#define XCC_LOG_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

bool
xcc_check_debug();

bool
xcc_check_trace();

void
trc(const char* format, ...);

void
dbg(const char* format, ...);

void
err(const char* format, ...);

#ifdef __cplusplus
}
#endif

#endif
