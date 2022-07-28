#ifndef XCC_ALGORITHM_H
#define XCC_ALGORITHM_H

#include "xcc.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct xcc_algorithm xcc_algorithm;

typedef const char* (*xcc_define_primary_item)(xcc_algorithm* a,
                                               xcc_problem* p,
                                               xcc_name n);
typedef const char* (*xcc_define_primary_item_with_range)(xcc_algorithm* a,
                                                          xcc_problem* p,
                                                          xcc_name n,
                                                          xcc_link slack,
                                                          xcc_link bound);

typedef const char* (*xcc_define_secondary_item)(xcc_algorithm* a,
                                                 xcc_problem* p,
                                                 xcc_name n);

typedef const char* (*xcc_add_item)(xcc_algorithm* a,
                                    xcc_problem* p,
                                    xcc_link l);
typedef const char* (*xcc_add_item_with_color)(xcc_algorithm* a,
                                               xcc_problem* p,
                                               xcc_link l,
                                               xcc_color color);
typedef const char* (*xcc_end_option)(xcc_algorithm* a, xcc_problem* p);
typedef const char* (*xcc_prepare_options)(xcc_algorithm* a, xcc_problem* p);
typedef const char* (*xcc_end_options)(xcc_algorithm* a, xcc_problem* p);

typedef const char* (*xcc_init_problem)(xcc_algorithm* a, xcc_problem* p);

typedef bool (*xcc_compute_next_result)(xcc_algorithm* a, xcc_problem* p);

typedef xcc_link (*xcc_choose_i)(xcc_algorithm* a, xcc_problem* p);

xcc_link
xcc_choose_i_naively(xcc_algorithm* a, xcc_problem* p);

xcc_link
xcc_choose_i_mrv(xcc_algorithm* a, xcc_problem* p);

typedef struct xcc_algorithm {
  xcc_define_primary_item define_primary_item;
  xcc_define_primary_item_with_range define_primary_item_with_range;
  xcc_define_secondary_item define_secondary_item;
  xcc_end_option end_option;
  xcc_prepare_options prepare_options;
  xcc_add_item add_item;
  xcc_add_item_with_color add_item_with_color;
  xcc_end_options end_options;

  xcc_init_problem init_problem;

  xcc_compute_next_result compute_next_result;

  xcc_choose_i choose_i;
} xcc_algorithm;

void
xcc_algorithm_standard_functions(xcc_algorithm* algorithm);

static const char*
xcc_default_init_problem(xcc_algorithm* a, xcc_problem* p);

bool
xcc_algorithm_from_select(int algorithm_select, xcc_algorithm* algorithm);

#ifdef __cplusplus
}
#endif

#endif
