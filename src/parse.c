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
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <xcc/parse.h>
#include <xcc/log.h>

#include <xcc_parser.h>

#include <xcc/xcc_lexer.h>

#include <xcc/algorithm_x.h>

extern FILE *yyin;

xcc_problem*
xcc_parse_problem(xcc_algorithm* a, const char* str) {
  int i;

  xcc_problem* problem = NULL;

  yyscan_t scanner;
  if(xcclex_init(&scanner))
    goto DESTROY_SCANNER;

  YY_BUFFER_STATE buf = xcc_scan_string(str, scanner);
  if(xccparse(scanner, a, &problem))
    goto DELETE_BUFFER;

  xcc_delete_buffer(buf, scanner);
  xcclex_destroy(scanner);

  return problem;

DELETE_BUFFER:
  xcc_delete_buffer(buf, scanner);
DESTROY_SCANNER:
  xcclex_destroy(scanner);

  if(problem)
    xcc_problem_free(problem, NULL);
  return NULL;
}

xcc_problem*
xcc_parse_problem_file(xcc_algorithm* a, const char* file) {
  int i;

  xcc_problem* problem = NULL;

  yyscan_t scanner;
  if(xcclex_init(&scanner))
    goto DESTROY_SCANNER;

  FILE* in = fopen(file, "rb");

  if(!in) {
    err("Could not open file %s!", file);
    return NULL;
  }

  xccset_in(in, scanner);

  if(xccparse(scanner, a, &problem))
    goto DESTROY_SCANNER;

  xcclex_destroy(scanner);

  return problem;

DESTROY_SCANNER:
  xcclex_destroy(scanner);

  if(problem)
    xcc_problem_free(problem, NULL);
  return NULL;
}
