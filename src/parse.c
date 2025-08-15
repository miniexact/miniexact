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
#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <miniexact/algorithm.h>
#include <miniexact/log.h>
#include <miniexact/miniexact.h>
#include <miniexact/parse.h>

extern FILE* yyin;

struct miniexact_parser;

typedef int (*miniexact_getc)(struct miniexact_parser* p);
typedef int (*miniexact_peekc)(struct miniexact_parser* p);

typedef const char* (*miniexact_add)(struct miniexact_parser* p,
                                     int lit,
                                     uint32_t cost);

typedef const char* (*miniexact_init_xc)(struct miniexact_parser* p,
                                         int primaries,
                                         int secondaries);
typedef const char* (*miniexact_init_miniexact)(struct miniexact_parser* p,
                                                int primaries,
                                                int secondaries);

typedef enum miniexact_dimacs_problem {
  DIMACS_XC,
  DIMACS_XCC
} miniexact_dimacs_problem;

typedef struct miniexact_parser {
  miniexact_problem* p;
  miniexact_algorithm* a;
  miniexact_getc mgetc;
  miniexact_peekc mpeekc;

  // Source for characters.
  FILE* file;
  const char* str;
  size_t str_pos;

  // Ident
  char ident[4096];
  size_t ident_len;

  size_t line;
  size_t col;
  size_t pos;

  int dimacs_primaries, dimacs_secondaries;
  miniexact_dimacs_problem dimacs_problem;
  int dimacs_last_lit;

  bool significant_newline;
} miniexact_parser;

typedef enum miniexact_token {
  END,
  IDENT,
  LBRACK,
  RBRACK,
  LESS_THAN,
  GREATER_THAN,
  COLON,
  SEMICOLON,
  DOLLAR,
  NEWLINE,
  PIPE,
} miniexact_token;

#define GETC(P) P->mgetc(P)
#define PEEKC(P) P->mpeekc(P)

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
isonlydigits(miniexact_parser* p) {
  for(size_t i = 0; i < p->ident_len; ++i)
    if(p->ident[i] > '9' || p->ident[i] < '0')
      return false;
  return true;
}

static miniexact_token
next(miniexact_parser* p) {
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
      if(p->significant_newline) {
        return NEWLINE;
      }
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
    case '$':
      return DOLLAR;
    case '|':
      return PIPE;
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
miniexact_getc_str(miniexact_parser* p) {
  if(p->str[p->str_pos] == '\0')
    return EOF;
  ++p->pos;
  return p->str[p->str_pos++];
}
static inline int
miniexact_peekc_str(miniexact_parser* p) {
  return p->str[p->str_pos];
}

static inline int
miniexact_getc_file(miniexact_parser* p) {
  ++p->pos;
  return getc(p->file);
}
static inline int
miniexact_peekc_file(miniexact_parser* p) {
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
parse_dimacs_kissat(miniexact_parser* p,
                    miniexact_init_xc xc,
                    miniexact_init_miniexact miniexact,
                    miniexact_add add) {
  int ch;
  uint64_t parsed = 0;
  int lit = 0;
  ch = GETC(p);
  uint32_t cost = 0;

  miniexact_init_xc init = NULL;

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
    init = miniexact;
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
    } else if(ch == '$') {
      ch = GETC(p);
      ++p->col;
      cost = ch - '0';
      while(ISDIGIT(ch = GETC(p))) {
        ++p->col;
        cost *= 10;
        const int digit = ch - '0';
        cost += digit;
      }
      continue;
    } else if(!ISDIGIT(ch))
      return "expected digit, $, or '-'";
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
    err = add(p, lit, cost);
    if(err)
      return err;
    if(lit == 0)
      cost = 0;
  }
  if(lit)
    return "trailing zero missing";

  return 0;
}

static const char*
parse_dimacs_init_xc(miniexact_parser* p, int primaries, int secondaries) {
  p->dimacs_problem = DIMACS_XC;
  p->dimacs_primaries = primaries;
  p->dimacs_secondaries = secondaries;

  if(primaries + secondaries > INT_MAX)
    return "primaries + secondaries must be smaller than INT_MAX";

  const char* e = NULL;
  for(int item = 1; item <= primaries; ++item) {
    if((e = p->a->define_primary_item(p->a, p->p, item)))
      return e;
    miniexact_append_NULL_to_name(p->p);
  }
  for(int item = 1; item <= secondaries; ++item) {
    if((e = p->a->define_secondary_item(p->a, p->p, item + primaries)))
      return e;
    miniexact_append_NULL_to_name(p->p);
  }

  if((e = p->a->prepare_options(p->a, p->p)))
    return e;

  return NULL;
}

static const char*
parse_dimacs_init_miniexact(miniexact_parser* p,
                            int primaries,
                            int secondaries) {
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
parse_dimacs_add(miniexact_parser* pp, int lit, uint32_t cost) {
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
    miniexact_problem* p = pp->p;
    miniexact_link color = -lit;
  } else if(lit < 0 && IS_PRIMARY(pp->dimacs_last_lit)) {
    return "primary items cannot be colored";
  } else if(lit == 0) {
    if(IS_SECONDARY(pp->dimacs_last_lit)) {
      if((e = pp->a->add_item(pp->a, pp->p, pp->dimacs_last_lit)))
        return e;
    }
    if((e = pp->a->end_option(pp->a, pp->p, cost)))
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
parse_dimacs(miniexact_parser* p) {
  // Multiple variants of Dimacs exist in the context of exact cover:
  // XC: Exact cover with Algorithm X.
  // XCC: Exact cover with colors

  p->dimacs_last_lit = 0;

  return parse_dimacs_kissat(
    p, &parse_dimacs_init_xc, &parse_dimacs_init_miniexact, &parse_dimacs_add);
}

static const char*
parse(miniexact_parser* p, bool dlx) {
  // Read problem header (primary items, secondary items), then read all
  // options.
  const char* e = NULL;

  miniexact_token t;
  t = next(p);

  bool skip_secondary = false;

  miniexact_link cost = 0;

  if(t == LESS_THAN && !dlx) {
    // Primary items
    t = next(p);
    while(t == IDENT) {
      miniexact_link item = miniexact_item_from_ident(p->p, p->ident);
      if(item != -1) {
        return "duplicate name for new primary item";
      }
      item = miniexact_insert_ident_as_name(p->p, p->ident);

      t = next(p);
      if(t == COLON) {
        // Triggers define_primary_item_with_range
        t = next(p);

        miniexact_link u = 1;
        miniexact_link v = 1;

        if(t != IDENT && isonlydigits(p))
          return "token after colon in range specifier must be a number";

        u = atoi(p->ident);

        t = next(p);

        if(t == SEMICOLON || t == COLON) {
          t = next(p);
          if(t != IDENT || !isonlydigits(p))
            return "token after first number and semicolon in range "
                   "specifier "
                   "must also be a "
                   "number";
          v = atoi(p->ident);
          t = next(p);
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
  } else if(!dlx && t == IDENT && p->ident_len == 1 && p->ident[0] == 'p') {
    // Use the dimacs format to parse this problem! There, no tokenizer is
    // used, so we exit from the general parsing function.
    return parse_dimacs(p);
  } else {
    // Must use Donald Knuth's DLX format.

    // Make newline significant
    p->significant_newline = true;

    // Skip regular secondaries
    skip_secondary = true;

    // First, read the primary items.

    while(t == IDENT) {
      miniexact_link item = miniexact_item_from_ident(p->p, p->ident);
      if(item != -1) {
        return "duplicate name for new primary item";
      }
      item = miniexact_insert_ident_as_name(p->p, p->ident);

      t = next(p);
      if(t == COLON) {
        // Triggers define_primary_item_with_range
        t = next(p);

        miniexact_link u = 1;
        miniexact_link v = 1;

        if(t != IDENT && isonlydigits(p))
          return "token after colon in range specifier must be a number";

        u = atoi(p->ident);

        t = next(p);

        if(t == SEMICOLON || t == COLON) {
          t = next(p);
          if(t != IDENT || !isonlydigits(p))
            return "token after first number and semicolon in range "
                   "specifier "
                   "must also be a "
                   "number";
          v = atoi(p->ident);
          t = next(p);
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

    // Skip the separator.
    if(t == PIPE) {
      t = next(p);
    }

    // Then, read the secondaries.
    while(t == IDENT) {
      miniexact_link item = miniexact_item_from_ident(p->p, p->ident);
      if(item != -1) {
        return "duplicate name for new secondary item";
      }
      item = miniexact_insert_ident_as_name(p->p, p->ident);

      if((e = p->a->define_secondary_item(p->a, p->p, item)))
        return e;

      t = next(p);
    }

    // Expect a newline.
    if(t == NEWLINE)
      t = next(p);
    else {
      return "expect newline after primary and secondary items in DLX header";
    }
  }
  if(t == LBRACK && !skip_secondary) {
    t = next(p);
    while(t == IDENT) {
      miniexact_link item = miniexact_item_from_ident(p->p, p->ident);
      if(item != -1) {
        return "duplicate name for new secondary item";
      }
      item = miniexact_insert_ident_as_name(p->p, p->ident);

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
      miniexact_link item = miniexact_item_from_ident(p->p, p->ident);
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

        miniexact_link color =
          miniexact_color_from_ident_or_insert(p->p, p->ident);
        t = next(p);

        if((e = p->a->add_item_with_color(p->a, p->p, item, color)))
          return e;
      } else {
        if((e = p->a->add_item(p->a, p->p, item)))
          return e;
      }

      if(t == DOLLAR) {
        t = next(p);
        if(t != IDENT || !isonlydigits(p)) {
          return "expected number after $ to define cost of option";
        }
        cost = atoi(p->ident);
        t = next(p);
      }

      if(t == SEMICOLON || t == END ||
         (p->significant_newline && t == NEWLINE)) {
        if((e = p->a->end_option(p->a, p->p, cost)))
          return e;
        ++p->p->option_count;
        cost = 0;
        t = next(p);
      }
    }
  }

  return p->a->end_options(p->a, p->p);
}

miniexact_problem*
miniexact_parse_problem(miniexact_algorithm* a,
                        const char* str,
                        const miniexact_config* cfg) {
  int i;
  bool dlx = cfg && cfg->parse_dlx;

  miniexact_parser p;
  memset(&p, 0, sizeof(p));
  const char* error = NULL;

  miniexact_problem* problem = miniexact_problem_allocate();
  if((error = miniexact_default_init_problem(a, problem)))
    goto ERROR;

  p.a = a;
  p.str = str;
  p.str_pos = 0;
  p.p = problem;
  p.file = NULL;
  p.ident_len = 0;

  p.mgetc = &miniexact_getc_str;
  p.mpeekc = &miniexact_peekc_str;

  if((error = parse(&p, dlx)))
    goto ERROR;

  return p.p;
ERROR:
  miniexact_problem_free(p.p, a);
  miniexact_err(
    "Parse error at %u:%u (pos %u) %s", p.line, p.col, p.pos, error);
  return NULL;
}

miniexact_problem*
miniexact_parse_problem_file(miniexact_algorithm* a,
                             const char* file_path,
                             const miniexact_config* cfg) {
  bool dlx = cfg && cfg->parse_dlx;

  FILE* f = fopen(file_path, "r");
  if(!f) {
    miniexact_err(
      "Could not open file %s, error: %s", file_path, strerror(errno));
    return NULL;
  }

  const char* error = NULL;

  miniexact_problem* problem = miniexact_problem_allocate();
  if((error = miniexact_default_init_problem(a, problem)))
    goto ERROR;

  miniexact_parser p;
  memset(&p, 0, sizeof(p));
  p.a = a;
  p.file = f;
  p.p = problem;
  p.str = NULL;
  p.str_pos = 0;
  p.ident_len = 0;
  p.significant_newline = false;

  p.mgetc = &miniexact_getc_file;
  p.mpeekc = &miniexact_peekc_file;

  if((error = parse(&p, dlx)))
    goto ERROR;

  if(problem->name_size == 0) {
    error = "no problem given, no idents parsed";
    goto ERROR;
  }

  fclose(f);
  return problem;
ERROR:
  assert(problem);
  miniexact_problem_free(problem, a);
  miniexact_err(
    "Parse error at %u:%u (pos %u) %s", p.line, p.col, p.pos, error);
  return NULL;
}
