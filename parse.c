#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parse.h"

#include <xcc_parser.h>

#include <xcc_lexer.h>

#include <algorithm_x.h>

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
    xcc_problem_free(problem);
  return NULL;
}
