/*
    miniexact - Toolset to solve exact cover problems and extensions
    Copyright (C) 2021-2023  Maximilian Heisinger

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU Affero General Public License as
    published by the Free Software Foundation, either version 3 of the
    License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Affero General Public License for more details.

    You should have received a copy of the GNU Affero General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include <miniexact/log.h>

static bool debug = false;
static bool trace = false;

bool
miniexact_check_debug() {
  if(debug)
    return true;
  static int debug = -1;
  if(debug == -1) {
    const char* miniexact_debug = getenv("MINIEXACT_DEBUG");
    debug = miniexact_debug != NULL;
  }
  return debug;
}

bool
miniexact_check_trace() {
  if(trace)
    return true;
  static int trace = -1;
  if(trace == -1) {
    const char* miniexact_trace = getenv("MINIEXACT_TRACE");
    trace = miniexact_trace != NULL;
  }
  return trace;
}

void
miniexact_dbg(const char* format, ...) {
  if(miniexact_check_debug()) {
    fprintf(stderr, "[MINIEXACT] [DEBUG] ");
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fputc('\n', stderr);
  }
}

void
miniexact_trc(const char* format, ...) {
  if(miniexact_check_trace()) {
    fprintf(stderr, "[MINIEXACT] [TRACE] ");
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fputc('\n', stderr);
  }
}

void
miniexact_err(const char* format, ...) {
#if _POSIX_C_SOURCE >= 199309L
  flockfile(stderr);
#endif
  fprintf(stderr, "[MINIEXACT] [ERROR] ");
  va_list args;
  va_start(args, format);
  vfprintf(stderr, format, args);
  va_end(args);
  fputc('\n', stderr);
#if _POSIX_C_SOURCE >= 199309L
  funlockfile(stderr);
#endif
}
