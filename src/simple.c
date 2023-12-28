#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <xcc/algorithm.h>
#include <xcc/algorithm_c.h>
#include <xcc/algorithm_m.h>
#include <xcc/algorithm_x.h>
#include <xcc/simple.h>
#include <xcc/util.h>
#include <xcc/xcc.h>

enum state {
  S_ADD_PRIMARY_ITEMS = 1u << 0u,
  S_ADD_SECONDARY_ITEMS = 1u << 1u,
  S_ADDING_OPTION = 1u << 2u,
  S_READY = 1u << 3u,
  S_SOLUTIONS_AVAILABLE = 1u << 4u,
  S_DONE = 1u << 5u,
};

struct xccs {
  struct xcc_algorithm a;
  struct xcc_problem p;
  enum state s;

  const char** names;
  const char** colors;
};

static struct xccs*
alloc_handle() {
  struct xccs* h = calloc(1, sizeof(struct xccs));
  h->s = S_ADD_PRIMARY_ITEMS;
  return h;
}

struct xccs*
xccs_init_x() {
  struct xccs* h = alloc_handle();
  xcc_algorithm_x_set(&h->a);
  xcc_default_init_problem(&h->a, &h->p);
  return h;
}

struct xccs*
xccs_init_c() {
  struct xccs* h = alloc_handle();
  xcc_algorithm_c_set(&h->a);
  xcc_default_init_problem(&h->a, &h->p);
  return h;
}

struct xccs*
xccs_init_m() {
  struct xccs* h = alloc_handle();
  xcc_algorithm_m_set(&h->a);
  xcc_default_init_problem(&h->a, &h->p);
  return h;
}

#define xstr(s) str(s)
#define str(s) #s

#define DIE(MESSAGE)                \
  fprintf(stderr, "%s\n", MESSAGE); \
  assert(false);                    \
  exit(1);

#define TRY(STMT)                                                        \
  do {                                                                   \
    const char* err = STMT;                                              \
    if(err) {                                                            \
      fprintf(stderr, "Error while executing %s: %s\n", str(STMT), err); \
      exit(1);                                                           \
    }                                                                    \
  } while(false);

static const char*
require_state(struct xccs* h, int s) {
  assert(h);
  char* error =
    (h->s == s || h->s & s) ? NULL : "xccs state does not match desired state!";
  return error;
}

// Define a new primary item. U and V give the slack. By default, these can both
// be 1.
int32_t
xccs_define_primary_item_with_slack(struct xccs* h,
                                    const char* name,
                                    unsigned int u,
                                    unsigned int v) {
  assert(h);
  assert(name);
  TRY(require_state(h, S_ADD_PRIMARY_ITEMS));
  xcc_link item = xcc_item_from_ident(&h->p, name);
  if(item != -1) {
    DIE("duplicate name for new primary item")
  }
  item = xcc_insert_ident_as_name(&h->p, name);
  TRY(h->a.define_primary_item_with_range(&h->a, &h->p, item, u, v));
  return h->p.i;
}

int32_t
xccs_define_primary_item(struct xccs* h, const char* name) {
  return xccs_define_primary_item_with_slack(h, name, 1, 1);
}

int32_t
xccs_define_secondary_item(struct xccs* h, const char* name) {
  assert(h);
  assert(name);
  TRY(require_state(h, S_ADD_PRIMARY_ITEMS | S_ADD_SECONDARY_ITEMS));
  if(h->s == S_ADD_PRIMARY_ITEMS) {
    h->s = S_ADD_SECONDARY_ITEMS;
  }

  xcc_link item = xcc_item_from_ident(&h->p, name);
  if(item != -1) {
    DIE("duplicate name for new secondary item")
  }
  item = xcc_insert_ident_as_name(&h->p, name);
  TRY(h->a.define_secondary_item(&h->a, &h->p, item));
  return h->p.i;
}

int32_t
xccs_define_color(struct xccs* h, const char* name) {
  assert(h);
  assert(name);
  return xcc_color_from_ident_or_insert(&h->p, name);
}

int32_t
xccs_add_named(struct xccs* h, const char* name, const char* color_str) {
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

    xcc_link item = xcc_item_from_ident(&h->p, name);
    if(color_str) {
      xcc_link color = xccs_define_color(h, color_str);
      TRY(h->a.add_item_with_color(&h->a, &h->p, item, color));
    } else {
      TRY(h->a.add_item(&h->a, &h->p, item));
    }
    return 0;
  } else {
    TRY(require_state(h, S_ADDING_OPTION));

    TRY(h->a.end_option(&h->a, &h->p));
    ++h->p.option_count;

    h->s = S_READY;
    return h->p.option_count - 1;
  }
}

int32_t
xccs_add(struct xccs* h, int32_t item, int32_t color) {
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

    TRY(h->a.end_option(&h->a, &h->p));
    ++h->p.option_count;

    h->s = S_READY;
    return h->p.option_count - 1;
  }
}

int
xccs_solve(struct xccs* h) {
  TRY(require_state(h, S_READY | S_SOLUTIONS_AVAILABLE));

  int r = xcc_solve_problem(&h->a, &h->p);
  if(r == 10) {
    h->s = S_SOLUTIONS_AVAILABLE;
  } else {
    h->s = S_DONE;
  }
  return r;
}

struct userdata_bag {
  struct xccs* h;
  xccs_solution_iterator it;
  void* userdata;
};

static void
it_converter(struct xcc_problem* p,
             const char** names,
             const char** colors,
             size_t items_count,
             void* userdata) {
  assert(userdata);
  struct userdata_bag* b = (struct userdata_bag*)userdata;
  b->it(b->h, names, colors, items_count, b->userdata);
}

void
xccs_solution(struct xccs* h, xccs_solution_iterator it, void* userdata) {
  TRY(require_state(h, S_SOLUTIONS_AVAILABLE));

  struct userdata_bag b = { .h = h, .it = it, .userdata = userdata };
  xcc_iterate_solution_options_str(&h->p, &it_converter, &b);
}

unsigned int
xccs_solution_length(struct xccs* h) {
  assert(h);
  return h->p.l;
}

int32_t
xccs_extract_solution(struct xccs* h, int32_t* arr) {
  assert(h);
  assert(arr);
  return xcc_extract_solution_option_indices(&h->p, arr);
}

void
xccs_free(struct xccs* h) {
  if(h) {
    xcc_problem_free_inner(&h->p, &h->a);
    free(h);
  }
}
