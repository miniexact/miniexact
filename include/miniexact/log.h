#ifndef MINIEXACT_LOG_H
#define MINIEXACT_LOG_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

bool
miniexact_check_debug(void);

bool
miniexact_check_trace(void);

void
miniexact_trc(const char* format, ...);

void
miniexact_dbg(const char* format, ...);

void
miniexact_err(const char* format, ...);

#ifdef __cplusplus
}
#endif

#endif
