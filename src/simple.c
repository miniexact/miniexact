#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <miniexact/algorithm.h>
#include <miniexact/algorithm_c.h>
#include <miniexact/algorithm_m.h>
#include <miniexact/algorithm_x.h>
#include <miniexact/miniexact.h>
#include <miniexact/simple.h>
#include <miniexact/util.h>

enum state {
  S_ADD_PRIMARY_ITEMS = 1u << 0u,
  S_ADD_SECONDARY_ITEMS = 1u << 1u,
  S_ADDING_OPTION = 1u << 2u,
  S_READY = 1u << 3u,
  S_SOLUTIONS_AVAILABLE = 1u << 4u,
  S_DONE = 1u << 5u,
  S_ERROR = 1u << 6u,
};

struct miniexacts {
  struct miniexact_algorithm a;
  struct miniexact_problem p;
  enum state s;

  const char** names;
  const char** colors;
};

static struct miniexacts*
alloc_handle() {
  struct miniexacts* h = calloc(1, sizeof(struct miniexacts));
  h->s = S_ADD_PRIMARY_ITEMS;
  return h;
}

struct miniexacts*
miniexacts_init_x() {
  struct miniexacts* h = alloc_handle();
  miniexact_algorithm_x_set(&h->a);
  miniexact_default_init_problem(&h->a, &h->p);
  return h;
}

struct miniexacts*
miniexacts_init_c() {
  struct miniexacts* h = alloc_handle();
  miniexact_algorithm_c_set(&h->a);
  miniexact_default_init_problem(&h->a, &h->p);
  return h;
}

struct miniexacts*
miniexacts_init_m() {
  struct miniexacts* h = alloc_handle();
  miniexact_algorithm_m_set(&h->a);
  miniexact_default_init_problem(&h->a, &h->p);
  return h;
}

#define xstr(s) str(s)
#define str(s) #s

#define DIE(MESSAGE)                                \
  fprintf(stderr, "!! FATAL ERROR: %s\n", MESSAGE); \
  h->s = S_ERROR;                                   \
  return 0;

#define TRY(STMT)                                                           \
  do {                                                                      \
    const char* err = STMT;                                                 \
    if(err) {                                                               \
      fprintf(stderr, "!! Error while executing %s: %s\n", str(STMT), err); \
      h->s = S_ERROR;                                                       \
      return 0;                                                             \
    }                                                                       \
  } while(false);

static const char*
require_state(struct miniexacts* h, int s) {
  assert(h);
  char* error = (h->s == s || h->s & s)
                  ? NULL
                  : "miniexacts state does not match desired state!";
  return error;
}

// Define a new primary item. U and V give the slack. By default, these can both
// be 1.
int32_t
miniexacts_define_primary_item_with_slack(struct miniexacts* h,
                                          const char* name,
                                          unsigned int u,
                                          unsigned int v) {
  assert(h);
  assert(name);
  TRY(require_state(h, S_ADD_PRIMARY_ITEMS));
  miniexact_link item = miniexact_item_from_ident(&h->p, name);
  if(item != -1) {
    DIE("duplicate name for new primary item")
  }
  item = miniexact_insert_ident_as_name(&h->p, name);
  TRY(h->a.define_primary_item_with_range(&h->a, &h->p, item, u, v));
  return h->p.i;
}

int32_t
miniexacts_define_primary_item(struct miniexacts* h, const char* name) {
  return miniexacts_define_primary_item_with_slack(h, name, 1, 1);
}

int32_t
miniexacts_define_secondary_item(struct miniexacts* h, const char* name) {
  assert(h);
  assert(name);
  TRY(require_state(h, S_ADD_PRIMARY_ITEMS | S_ADD_SECONDARY_ITEMS));
  if(h->s == S_ADD_PRIMARY_ITEMS) {
    h->s = S_ADD_SECONDARY_ITEMS;
  }

  miniexact_link item = miniexact_item_from_ident(&h->p, name);
  if(item != -1) {
    DIE("duplicate name for new secondary item")
  }
  item = miniexact_insert_ident_as_name(&h->p, name);
  TRY(h->a.define_secondary_item(&h->a, &h->p, item));
  return h->p.i;
}

int32_t
miniexacts_define_color(struct miniexacts* h, const char* name) {
  assert(h);
  assert(name);
  return miniexact_color_from_ident_or_insert(&h->p, name);
}

int32_t
miniexacts_add_named(struct miniexacts* h,
                     const char* name,
                     const char* color_str,
                     uint32_t cost) {
  assert(h);

  if(name)
    if(strcmp(name, "") == 0)
      name = NULL;
  if(color_str)
    if(strcmp(color_str, "") == 0)
      color_str = NULL;

  if(h->s == S_ADD_PRIMARY_ITEMS || h->s == S_ADD_SECONDARY_ITEMS) {
    TRY(h->a.prepare_options(&h->a, &h->p));
    h->s = S_READY;
  }

  if(name) {
    TRY(require_state(h, S_READY | S_ADDING_OPTION));
    h->s = S_ADDING_OPTION;

    miniexact_link item = miniexact_item_from_ident(&h->p, name);
    if(color_str) {
      miniexact_link color = miniexacts_define_color(h, color_str);
      TRY(h->a.add_item_with_color(&h->a, &h->p, item, color));
    } else {
      TRY(h->a.add_item(&h->a, &h->p, item));
    }
    return 0;
  } else {
    TRY(require_state(h, S_ADDING_OPTION));

    TRY(h->a.end_option(&h->a, &h->p, cost));
    ++h->p.option_count;

    h->s = S_READY;
    return h->p.option_count;
  }
}

int32_t
miniexacts_add(struct miniexacts* h,
               int32_t item,
               int32_t color,
               uint32_t cost) {
  assert(h);

  if(h->s == S_ADD_PRIMARY_ITEMS || h->s == S_ADD_SECONDARY_ITEMS) {
    TRY(h->a.prepare_options(&h->a, &h->p));
    h->s = S_READY;
  }

  if(item) {
    TRY(require_state(h, S_READY | S_ADDING_OPTION));
    h->s = S_ADDING_OPTION;

    if(color) {
      TRY(h->a.add_item_with_color(&h->a, &h->p, item, color));
    } else {
      TRY(h->a.add_item(&h->a, &h->p, item));
    }
    return 0;
  } else {
    TRY(require_state(h, S_ADDING_OPTION));

    TRY(h->a.end_option(&h->a, &h->p, cost));
    ++h->p.option_count;

    h->s = S_READY;
    return h->p.option_count;
  }
}

int
miniexacts_solve(struct miniexacts* h) {
  TRY(require_state(h, S_READY | S_SOLUTIONS_AVAILABLE));

  int r = miniexact_solve_problem(&h->a, &h->p);
  if(r == 10) {
    h->s = S_SOLUTIONS_AVAILABLE;
  } else {
    h->s = S_DONE;
  }
  return r;
}

struct userdata_bag {
  struct miniexacts* h;
  miniexacts_solution_iterator it;
  void* userdata;
};

static void
it_converter(struct miniexact_problem* p,
             const char** names,
             const char** colors,
             size_t items_count,
             void* userdata) {
  assert(userdata);
  struct userdata_bag* b = (struct userdata_bag*)userdata;
  b->it(b->h, names, colors, items_count, b->userdata);
}

bool
miniexacts_solution(struct miniexacts* h,
                    miniexacts_solution_iterator it,
                    void* userdata) {
  TRY(require_state(h, S_SOLUTIONS_AVAILABLE));

  struct userdata_bag b = { .h = h, .it = it, .userdata = userdata };
  miniexact_iterate_solution_options_str(&h->p, &it_converter, &b);
  return true;
}

unsigned int
miniexacts_solution_length(struct miniexacts* h) {
  assert(h);
  return h->p.l;
}

int32_t
miniexacts_extract_solution(struct miniexacts* h, int32_t* arr) {
  assert(h);
  assert(arr);
  return miniexact_extract_solution_option_indices(&h->p, arr);
}

bool
miniexacts_write_to_dlx(struct miniexacts* h, const char* path) {
  assert(h);
  assert(path);
  FILE* out = fopen(path, "w");
  miniexact_write_problem_to_dlx(&h->p, out);
  fclose(out);
  return true;
}

miniexact_problem*
miniexacts_problem(struct miniexacts* h) {
  return &h->p;
}

void
miniexacts_free(struct miniexacts* h) {
  if(h) {
    miniexact_problem_free_inner(&h->p, &h->a);
    free(h);
  }
}
