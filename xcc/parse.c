#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "parse.h"

#include <xcc_parser.h>


int xcc_parse_problem() {
  int i;

  xcc_algorithm algorithm;
  xcc_problem *problem = NULL;
  memset(&algorithm, 0, sizeof(xcc_algorithm));

  xcc_yy_parse_string("test", &algorithm, &problem);


  return 0;
}
