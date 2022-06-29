#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>

#include "parse.h"

#define YYSTYPE XCCTYPE
#include <xcc_parser.h>


int xcc_parse_problem() {
  int i;

  yyscan_t scanner;

  xcc_yy_parse_string("test", &scanner);

  if((i = xcclex_init(&scanner)) != 0)
    return EXIT_FAILURE;

  xcc_algorithm algorithm;
  xcc_problem *problem = NULL;

  int e = xccparse(&algorithm, &problem, &scanner);
  printf("Code = %d\n", e);
  if(e == 0 /* success */) {
  }

  xcclex_destroy(scanner);
  return 0;
}
