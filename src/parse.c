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
#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <xcc/algorithm.h>
#include <xcc/log.h>
#include <xcc/parse.h>
#include <xcc/xcc.h>

extern FILE* yyin;

struct xcc_parser;

typedef int (*xcc_getc)(struct xcc_parser* p);
typedef int (*xcc_peekc)(struct xcc_parser* p);

typedef const char* (*xcc_add)(struct xcc_parser* p, int lit);

typedef const char* (*xcc_init_xc)(struct xcc_parser* p,
                                   int primaries,
                                   int secondaries);
typedef const char* (*xcc_init_xcc)(struct xcc_parser* p,
                                    int primaries,
                                    int secondaries);

typedef enum xcc_dimacs_problem { DIMACS_XC, DIMACS_XCC } xcc_dimacs_problem;

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

  int dimacs_primaries, dimacs_secondaries;
  xcc_dimacs_problem dimacs_problem;
  int dimacs_last_lit;
} xcc_parser;

typedef enum xcc_token {
  END,
  IDENT,
  LBRACK,
  RBRACK,
  LESS_THAN,
  GREATER_THAN,
  COLON,
  SEMICOLON,
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

static inline bool
faster_is_digit(int ch) {
  return '0' <= ch && ch <= '9';
}

#define ISDIGIT(CH) faster_is_digit(CH)

// The dimacs parser is taken from Kissat, by Armin Biere (MIT Licensed) and
// modified to parse exact cover header and options.
static const char*
parse_dimacs_kissat(xcc_parser* p,
                    xcc_init_xc xc,
                    xcc_init_xcc xcc,
                    xcc_add add) {
  int ch;
  uint64_t parsed = 0;
  int lit = 0;
  ch = GETC(p);

  xcc_init_xc init = NULL;

  if(ch != ' ')
    return "expected space after 'p'";
  ch = GETC(p);
  ++p->col;
  if(ch != 'x')
    return "expected 'x' after 'p '";
  ch = GETC(p);
  ++p->col;
  if(ch != 'c')
    return "expected 'c' after 'p x'";
  ch = GETC(p);
  ++p->col;
  if(ch == 'c') {
    init = xcc;
    ch = GETC(p);
    ++p->col;
  } else
    init = xc;
  if(ch != ' ')
    return "expected space after 'p xc'";
  ch = GETC(p);
  ++p->col;
  if(!ISDIGIT(ch))
    return "expected digit after 'p cnf '";
  int primaries = ch - '0';
  while(ISDIGIT(ch = GETC(p))) {
    ++p->col;
    if(UINT64_MAX / 10 < primaries)
      return "primaries too large";
    primaries *= 10;
    const int digit = ch - '0';
    primaries += digit;
  }
  if(ch == EOF)
    return "unexpected end-of-file while parsing digit";
  if(ch == '\r') {
    ch = GETC(p);
    ++p->col;
    if(ch != '\n')
      return "expected new-line after carriage-return";
  }
  if(ch == '\n')
    return "unexpected new-line after primaries";
  if(ch != ' ')
    return "expected space after primaries";
  ch = GETC(p);
  ++p->col;
  if(!ISDIGIT(ch))
    return "expected secondaries after primaries";
  uint64_t secondaries = ch - '0';
  while(ISDIGIT(ch = GETC(p))) {
    ++p->col;
    if(INT_MAX / 10 < secondaries)
      return "secondaries too large";
    secondaries *= 10;
    const int digit = ch - '0';
    if(INT_MAX - digit < secondaries)
      return "secondaries too large";
    secondaries += digit;
  }
  if(ch == EOF)
    return "unexpected end-of-file while parsing secondaries";
  if(ch == '\r') {
    ch = GETC(p);
    if(ch != '\n')
      return "expected new-line after carriage-return";
  }
  if(ch == EOF)
    return "unexpected end-of-file after parsing secondaries";
  if(ch != '\n')
    return "expected new-line after parsing secondaries";

  p->col = 0;
  ++p->line;

  const char* err = init(p, primaries, secondaries);
  if(err)
    return err;

  for(;;) {
    ch = GETC(p);
    ++p->col;
    if(ch == ' ')
      continue;
    if(ch == '\t')
      continue;
    if(ch == '\n') {
      p->col = 0;
      ++p->line;
      continue;
    }
    if(ch == '\r') {
      ch = GETC(p);
      if(ch != '\n')
        return "expected new-line after carriage-return";
      continue;
    }
    if(ch == EOF)
      break;
    int sign;
    if(ch == '-') {
      ch = GETC(p);
      ++p->col;
      if(ch == EOF)
        return "unexpected end-of-file after '-'";
      if(ch == '\n')
        return "unexpected new-line after '-'";
      if(!ISDIGIT(ch))
        return "expected digit after '-'";
      if(ch == '0')
        return "expected non-zero digit after '-'";
      sign = -1;
    } else if(!ISDIGIT(ch))
      return "expected digit or '-'";
    else
      sign = 1;
    assert(ISDIGIT(ch));
    int idx = ch - '0';
    while(ISDIGIT(ch = GETC(p))) {
      ++p->col;
      idx *= 10;
      const int digit = ch - '0';
      idx += digit;
    }
    if(ch == EOF) {
      if(idx)
        return "unexpected end-of-file after literal";
      else
        return "unexpected end-of-file after trailing zero";
    } else if(ch == '\r') {
      ch = GETC(p);
      ++p->col;
      if(ch != '\n')
        return "expected new-line after carriage-return";
      p->col = 0;
      ++p->line;
    } else if(ch != ' ' && ch != '\t' && ch != '\n')
      return "expected white space after literal";
    if(idx) {
      assert(sign == 1 || sign == -1);
      assert(idx != INT_MIN);
      lit = sign * idx;
    } else {
      parsed++;
      lit = 0;
    }
    err = add(p, lit);
    if(err)
      return err;
  }
  if(lit)
    return "trailing zero missing";

  return 0;
}

static const char*
parse_dimacs_init_xc(xcc_parser* p, int primaries, int secondaries) {
  p->dimacs_problem = DIMACS_XC;
  p->dimacs_primaries = primaries;
  p->dimacs_secondaries = secondaries;

  if(primaries + secondaries > INT_MAX)
    return "primaries + secondaries must be smaller than INT_MAX";

  const char* e = NULL;
  for(int item = 1; item <= primaries; ++item) {
    if((e = p->a->define_primary_item(p->a, p->p, item)))
      return e;
    xcc_append_NULL_to_name(p->p);
  }
  for(int item = 1; item <= secondaries; ++item) {
    if((e = p->a->define_secondary_item(p->a, p->p, item + primaries)))
      return e;
    xcc_append_NULL_to_name(p->p);
  }

  if((e = p->a->prepare_options(p->a, p->p)))
    return e;

  return NULL;
}

static const char*
parse_dimacs_init_xcc(xcc_parser* p, int primaries, int secondaries) {
  const char* e = parse_dimacs_init_xc(p, primaries, secondaries);
  if(e)
    return e;

  p->dimacs_problem = DIMACS_XCC;
  return NULL;
}

#define IS_PRIMARY(L) (L > 0 && L <= pp->dimacs_primaries)
#define IS_SECONDARY(L)        \
  (L > pp->dimacs_primaries && \
   L <= pp->dimacs_primaries + pp->dimacs_secondaries)

static const char*
parse_dimacs_add(xcc_parser* pp, int lit) {
  const char* e = NULL;

  if(IS_PRIMARY(lit)) {
    if(IS_SECONDARY(pp->dimacs_last_lit)) {
      if((e = pp->a->add_item(pp->a, pp->p, pp->dimacs_last_lit)))
        return e;
    }
    if((e = pp->a->add_item(pp->a, pp->p, lit)))
      return e;
  } else if(IS_SECONDARY(lit)) {
    if(IS_SECONDARY(pp->dimacs_last_lit)) {
      if((e = pp->a->add_item(pp->a, pp->p, pp->dimacs_last_lit)))
        return e;
    }
  } else if(lit < 0 && IS_SECONDARY(pp->dimacs_last_lit)) {
    if((e =
          pp->a->add_item_with_color(pp->a, pp->p, pp->dimacs_last_lit, -lit)))
      return e;
    xcc_problem* p = pp->p;
    xcc_link color = -lit;
    XCC_ARR_HASN(color_name, color);
    p->color_name[color] = NULL;
  } else if(lit < 0 && IS_PRIMARY(pp->dimacs_last_lit)) {
    return "primary items cannot be colored";
  } else if(lit == 0) {
    if(IS_SECONDARY(pp->dimacs_last_lit)) {
      if((e = pp->a->add_item(pp->a, pp->p, pp->dimacs_last_lit)))
        return e;
    }
    if((e = pp->a->end_option(pp->a, pp->p)))
      return e;
  } else {
    return "invalid lit";
  }

  pp->dimacs_last_lit = lit;

  return NULL;
}

#undef IS_PRIMARY
#undef IS_SECONDARY

static const char*
parse_dimacs(xcc_parser* p) {
  // Multiple variants of Dimacs exist in the context of exact cover:
  // XC: Exact cover with Algorithm X.
  // XCC: Exact cover with colors

  p->dimacs_last_lit = 0;

  return parse_dimacs_kissat(
    p, &parse_dimacs_init_xc, &parse_dimacs_init_xcc, &parse_dimacs_add);
}

static const char*
parse(xcc_parser* p) {
  // Read problem header (primary items, secondary items), then read all
  // options.
  const char* e = NULL;

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
      if(t == COLON) {
        // Triggers define_primary_item_with_range
        t = next(p);

        xcc_link u = 1;
        xcc_link v = 1;

        if(t != IDENT && isonlydigits(p))
          return "token after colon in range specifier must be a number";

        u = atoi(p->ident);

        t = next(p);

        if(t == SEMICOLON) {
          t = next(p);
          if(t != IDENT || !isonlydigits(p))
            return "token after first number and semicolon in range specifier "
                   "must also be a "
                   "number";
          v = atoi(p->ident);
          t = next(p);
        } else if(t == COLON) {
          return "separate range specifiers with a semicolon";
        } else {
          // The range will be the first number specified by default.
          v = u;
        }
        if((e = p->a->define_primary_item_with_range(p->a, p->p, item, u, v)))
          return e;
      } else {
        if((e = p->a->define_primary_item(p->a, p->p, item)))
          return e;
      }
    }
    if(t != GREATER_THAN) {
      return "primary item list must end with >";
    }
    t = next(p);
  } else if(t == IDENT && p->ident_len == 1 && p->ident[0] == 'p') {
    // Use the dimacs format to parse this problem! There, no tokenizer is used,
    // so we exit from the general parsing function.
    return parse_dimacs(p);
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

      if((e = p->a->define_secondary_item(p->a, p->p, item)))
        return e;

      t = next(p);
    }
    if(t != RBRACK) {
      return "secondary item list must end with ]";
    }
    t = next(p);
  }

  if((e = p->a->prepare_options(p->a, p->p)))
    return e;

  while(t != END) {
    if(t != IDENT) {
      return "expected ident as option start";
    }
    while(t == IDENT) {
      xcc_link item = xcc_item_from_ident(p->p, p->ident);
      if(item == -1) {
        return p->ident;
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

        if((e = p->a->add_item_with_color(p->a, p->p, item, color)))
          return e;
      } else {
        if((e = p->a->add_item(p->a, p->p, item)))
          return e;
      }

      if(t == SEMICOLON || t == END) {
        if((e = p->a->end_option(p->a, p->p)))
          return e;
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
