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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <miniexact/algorithm.h>
#include <miniexact/log.h>
#include <miniexact/miniexact.h>
#include <miniexact/ops.h>

miniexact_problem*
miniexact_problem_allocate() {
  miniexact_problem* p = calloc(1, sizeof(miniexact_problem));
  p->K = 1;// Generate one solution by default.
  return p;
}

int
miniexact_search_for_name(const char* needle,
                          const miniexact_name* names,
                          size_t names_size) {
  for(int i = 0; i < names_size; ++i) {
    if(names[i] && strcmp(names[i], needle) == 0) {
      return i;
    }
  }
  return -1;
}

bool
miniexact_has_item(miniexact_link needle, miniexact_link* list, size_t len) {
  for(size_t i = 0; i < len; ++i) {
    if(list[i] == needle)
      return true;
  }
  return false;
}

void
miniexact_problem_free_inner(miniexact_problem* p, miniexact_algorithm* a) {
  if(p->algorithm_userdata && a && a->free_userdata)
    a->free_userdata(a, p);

  if(p->llink)
    free(p->llink);
  if(p->rlink)
    free(p->rlink);
  if(p->ulink)
    free(p->ulink);
  if(p->dlink)
    free(p->dlink);
  if(p->top)
    free(p->top);
  if(p->color)
    free(p->color);
  if(p->color_name) {
    for(size_t i = 0; i < p->color_name_size; ++i)
      if(p->color_name[i])
        free(p->color_name[i]);
    free(p->color_name);
  }
  if(p->name) {
    for(size_t i = 0; i < p->name_size; ++i)
      if(p->name[i])
        free(p->name[i]);
    free(p->name);
  }
  if(p->x)
    free(p->x);
  if(p->ft)
    free(p->ft);
  if(p->slack)
    free(p->slack);
  if(p->bound)
    free(p->bound);
  if(p->cost)
    free(p->cost);
  if(p->best)
    free(p->best);
  if(p->tho)
    free(p->tho);
  if(p->th)
    free(p->th);

  memset(p, 0, sizeof(miniexact_problem));
}

void
miniexact_problem_free(miniexact_problem* p, miniexact_algorithm* a) {
  miniexact_problem_free_inner(p, a);
  free(p);
}

miniexact_link
miniexact_item_from_ident(miniexact_problem* p, const char* ident) {
  return miniexact_search_for_name(ident, p->name, p->name_size);
}

miniexact_link
miniexact_insert_ident_as_name(miniexact_problem* p, const char* ident) {
  miniexact_link l = p->name_size;
  MINIEXACT_ARR_PLUS1(name)
  p->name[l] = strdup(ident);
  return l;
}

void
miniexact_append_NULL_to_name(miniexact_problem* p) {
  miniexact_link l = p->name_size;
  MINIEXACT_ARR_PLUS1(name)
  p->name[l] = NULL;
}

miniexact_link
miniexact_color_from_ident(miniexact_problem* p, const char* ident) {
  return miniexact_search_for_name(ident, p->color_name, p->color_name_size);
}

miniexact_link
miniexact_color_from_ident_or_insert(miniexact_problem* p, const char* ident) {
  miniexact_link l = miniexact_color_from_ident(p, ident);
  if(l == -1) {
    l = p->color_name_size;
    MINIEXACT_ARR_PLUS1(color_name)
    p->color_name[l] = strdup(ident);
  }
  return l;
}

void
miniexact_print_problem_matrix(miniexact_problem* p) {
  // Print Header first
  for(size_t i = 0; i < p->name_size; ++i) {
    if(i > 0 && i <= p->primary_item_count) {
      printf("i:%zu\tNAME:%s\tLLINK:%d\tRLINK:%d\tSLACK:%d\tBOUND:%d\n",
             i,
             p->name[i],
             p->llink[i],
             p->rlink[i],
             p->slack[i],
             p->bound[i]);
    } else {
      printf("i:%zu\tNAME:%s\tLLINK:%d\tRLINK:%d\n",
             i,
             p->name[i],
             p->llink[i],
             p->rlink[i]);
    }
  }
  printf("\n");
  for(size_t x = 0; x < p->ulink_size; ++x) {
    printf("x:%zu\tTOP/LEN:%d\tULINK:%d\tDLINK:%d\tCOLOR:%d\n",
           x,
           p->top[x],
           p->ulink[x],
           p->dlink[x],
           p->color[x]);
  }
}

const char*
miniexact_print_problem_matrix_in_libexact_format(miniexact_problem* p) {
  if(p->color_size)
    return "Colors not supported in libexact format!";
  if(p->secondary_item_count)
    return "Secondary items not supported in libexact format!";

  for(miniexact_link i = 1; i <= p->primary_item_count; ++i) {
    printf("# %s %d\n", NAME(i), i);
  }

  for(size_t i = 1; i <= p->primary_item_count; ++i) {
    printf("r %zu\n", i);
  }
  for(size_t i = 1; i <= p->option_count; ++i) {
    printf("c %zu\n", i);
  }

  for(miniexact_link i = p->N_1 + 1, option = 0;
      option < p->option_count && i < p->ulink_size;
      ++i) {
    if(TOP(i) <= 0)
      ++option;
    else {
      printf("e %d %d\n", TOP(i), option);
    }
  }

  return NULL;
}

static inline void
print_name(miniexact_problem* p, FILE* o, size_t i) {
  if(p->name[i] && p->name[i][0] != '0') {
    fprintf(o, "%s", p->name[i]);
  } else {
    fprintf(o, "%zu", i);
  }
}

static inline void
print_color(miniexact_problem* p, FILE* o, miniexact_link i) {
  if(p->color_name[i]) {
    fprintf(o, "%s", p->color_name[i]);
  } else {
    fprintf(o, "%d", i);
  }
}

void
miniexact_write_problem_to_dlx(miniexact_problem* p, FILE* o) {
  bool first = true;
  for(size_t i = 1; i < p->name_size; ++i) {
    if(i > 0 && i <= p->primary_item_count) {
      if(first) {
        first = false;
      } else {
        fprintf(o, " ");
      }
      print_name(p, o, i);
      if(p->slack[i] != 0) {
        fprintf(o, ":%d;%d", p->slack[i] + p->bound[i], p->bound[i]);
      }
    } else {
      if(!first) {
        first = true;
        fprintf(o, " |");
      }
      fprintf(o, " ");
      print_name(p, o, i);
    }
  }

  fprintf(o, "\n");

  first = true;
  for(miniexact_link i = p->N + 2; i < p->ulink_size; ++i) {
    if(first)
      first = false;
    else
      fprintf(o, " ");

    if(TOP(i) <= 0) {
      fprintf(o, "\n");
      first = true;
    } else {
      print_name(p, o, TOP(i));
      if(COLOR(i)) {
        fprintf(o, ":");
        print_color(p, o, COLOR(i));
      }
    }
  }
}

void
miniexact_print_problem_solution(miniexact_problem* p) {
  printf("Solution:\n");
  for(size_t i = 0; i < p->x_size; ++i) {
    printf("%d ", p->x[i]);
  }
  printf("\n");
}

miniexact_link
miniexact_extract_solution_option_indices(miniexact_problem* p,
                                          miniexact_link* solution) {
  assert(p);
  assert(solution);

  miniexact_link size = 0;

  for(miniexact_link j = 0; j < p->l; ++j) {
    miniexact_link r = p->x[j];

    if(r <= p->N || r > p->Z)
      continue;// Out of range (e.g. MCC)

    while(TOP(r) >= 0)
      ++r;
    solution[size++] = -TOP(r);
  }

  return size;
}
