#ifndef MINIEXACT_LOG_H
#define MINIEXACT_LOG_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

bool
miniexact_check_debug();

bool
miniexact_check_trace();

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
