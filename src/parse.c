/*
    XCCSolve - Toolset to solve exact cover problems and extensions
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
#include "xcc/xcc.h"
#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <xcc/algorithm.h>
#include <xcc/log.h>
#include <xcc/parse.h>

extern FILE* yyin;

struct xcc_parser;

typedef int (*xcc_getc)(struct xcc_parser* p);
typedef int (*xcc_peekc)(struct xcc_parser* p);

typedef struct xcc_parser {
  xcc_problem* p;
  xcc_algorithm* a;
  xcc_getc getc;
  xcc_peekc peekc;

  // Source for characters.
  FILE* file;
  const char* str;
  size_t str_pos;

  // Ident
  char ident[4096];
  size_t ident_len;

  size_t line;
  size_t col;
} xcc_parser;

typedef enum xcc_token {
  END,
  IDENT,
  LBRACK,
  RBRACK,
  LESS_THAN,
  GREATER_THAN,
  COLON,
  SEMICOLON
} xcc_token;

#define GETC(P) P->getc(P)
#define PEEKC(P) P->peekc(P)

inline static bool
isidentchar(char c) {
  return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
         (c >= '0' && c <= '9') || (c == '_') || (c == '-') || (c == ',') ||
         (c == '!') || (c == '#') || (c == '@') || (c == '+') || (c == '%') ||
         (c == '^') || (c == '&') || (c == '*') || (c & 128u);
  /* c & 128u checks for some UTF-8 extended character, we want to accept all of
     them for trivial unicode supporting behavior for idents */
}

inline static bool
isonlydigits(xcc_parser* p) {
  for(size_t i = 0; i < p->ident_len; ++i)
    if(p->ident[i] > '9' || p->ident[i] < '0')
      return false;
  return true;
}

static xcc_token
next(xcc_parser* p) {
  int c = 0;
  while(true) {
    c = GETC(p);
    if(c == EOF) {
      return END;
    }

    ++p->col;
    if(c == '\n') {
      p->col = 0;
      ++p->line;
      continue;
    }

    if(!isspace(c))
      break;
  }

  switch(c) {
    case ';':
      return SEMICOLON;
    case ':':
      return COLON;
    case '[':
      return LBRACK;
    case ']':
      return RBRACK;
    case '<':
      return LESS_THAN;
    case '>':
      return GREATER_THAN;
  }

  if(isidentchar(c)) {
    p->ident[0] = c;
    p->ident_len = 1;
    while(PEEKC(p) != EOF && isidentchar(PEEKC(p))) {
      char c = GETC(p);
      ++p->col;
      p->ident[p->ident_len++] = c;
    }
    p->ident[p->ident_len] = '\0';
    return IDENT;
  }

  return END;
}

static inline int
xcc_getc_str(xcc_parser* p) {
  if(p->str[p->str_pos] == '\0')
    return EOF;
  return p->str[p->str_pos++];
}
static inline int
xcc_peekc_str(xcc_parser* p) {
  return p->str[p->str_pos];
}

static inline int
xcc_getc_file(xcc_parser* p) {
  return getc(p->file);
}
static inline int
xcc_peekc_file(xcc_parser* p) {
  // Thanks to https://stackoverflow.com/a/7623338
  const int c = getc(p->file);
  return c == EOF ? EOF : ungetc(c, p->file);
}

static const char*
parse(xcc_parser* p) {
  // Read problem header (primary items, secondary items), then read all
  // options.

  xcc_token t;
  t = next(p);

  if(t == LESS_THAN) {
    // Primary items
    t = next(p);
    while(t == IDENT) {
      xcc_link item = xcc_item_from_ident(p->p, p->ident);
      if(item != -1) {
        return "duplicate name for new primary item";
      }
      item = xcc_insert_ident_as_name(p->p, p->ident);

      t = next(p);
      xcc_link slack = 0;
      xcc_link bound = 0;
      if(t == COLON) {
        // Triggers define_primary_item_with_range
        t = next(p);

        xcc_link slack = 0;
        xcc_link bound = 1;

        t = next(p);

        if(t != IDENT && isonlydigits(p))
          return "token after colon in range specifier must be a number";

        slack = atoi(p->ident);

        t = next(p);

        if(t == SEMICOLON) {
          t = next(p);
          if(t != IDENT && isonlydigits(p))
            return "token after first number and semicolon in range specifier "
                   "must also be a "
                   "number";
          bound = atoi(p->ident);
        }
        p->a->define_primary_item_with_range(p->a, p->p, item, slack, bound);
      } else {
        p->a->define_primary_item(p->a, p->p, item);
      }
    }
    if(t != GREATER_THAN) {
      return "primary item list must end with >";
    }
    t = next(p);
  } else {
    return "no primary item definitions given";
  }
  if(t == LBRACK) {
    t = next(p);
    while(t == IDENT) {
      xcc_link item = xcc_item_from_ident(p->p, p->ident);
      if(item != -1) {
        return "duplicate name for new secondary item";
      }
      item = xcc_insert_ident_as_name(p->p, p->ident);

      p->a->define_secondary_item(p->a, p->p, item);

      t = next(p);
    }
    if(t != RBRACK) {
      return "secondary item list must end with ]";
    }
    t = next(p);
  }

  p->a->prepare_options(p->a, p->p);

  while(t != END) {
    if(t != IDENT) {
      return "expected ident as option start";
    }
    while(t == IDENT) {
      xcc_link item = xcc_item_from_ident(p->p, p->ident);
      if(item == -1) {
        return "encountered undefined item";
      }
      t = next(p);
      if(t == COLON) {
        t = next(p);

        if(t != IDENT)
          return "item : color, the color was not an ident";

        if(item <= p->p->primary_item_count) {
          return "cannot specify a color for a primary item";
        }

        xcc_link color = xcc_color_from_ident_or_insert(p->p, p->ident);
        t = next(p);

        p->a->add_item_with_color(p->a, p->p, item, color);
      } else {
        p->a->add_item(p->a, p->p, item);
      }

      if(t == SEMICOLON) {
        p->a->end_option(p->a, p->p);
        ++p->p->option_count;
        t = next(p);
      }
    }
  }

  return p->a->end_options(p->a, p->p);
}

xcc_problem*
xcc_parse_problem(xcc_algorithm* a, const char* str) {
  int i;

  xcc_parser p;
  memset(&p, 0, sizeof(p));
  const char* error = NULL;

  xcc_problem* problem = xcc_problem_allocate();
  if((error = xcc_default_init_problem(a, problem)))
    goto ERROR;

  p.a = a;
  p.str = str;
  p.str_pos = 0;
  p.p = problem;
  p.file = NULL;
  p.ident_len = 0;

  p.getc = &xcc_getc_str;
  p.peekc = &xcc_peekc_str;

  if((error = parse(&p)))
    goto ERROR;

  return p.p;
ERROR:
  xcc_problem_free(p.p, a);
  err("Parse error at %u:%u %s", p.line, p.col, error);
  return NULL;
}

xcc_problem*
xcc_parse_problem_file(xcc_algorithm* a, const char* file_path) {

  FILE* f = fopen(file_path, "r");
  if(!f) {
    err("Could not open file %s, error: %s", file_path, strerror(errno));
    return NULL;
  }

  const char* error = NULL;

  xcc_problem* problem = xcc_problem_allocate();
  if((error = xcc_default_init_problem(a, problem)))
    goto ERROR;

  xcc_parser p;
  memset(&p, 0, sizeof(p));
  p.a = a;
  p.file = f;
  p.p = problem;
  p.str = NULL;
  p.str_pos = 0;
  p.ident_len = 0;

  p.getc = &xcc_getc_file;
  p.peekc = &xcc_peekc_file;

  if((error = parse(&p)))
    goto ERROR;

  if(problem->name_size == 0) {
    error = "no problem given, no idents parsed";
    goto ERROR;
  }

  return problem;
ERROR:
  assert(problem);
  xcc_problem_free(problem, a);
  err("Parse error at %u:%u %s", p.line, p.col, error);
  return NULL;
}
