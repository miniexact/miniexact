/* An XCC parser.
 */

/* Following a guide here:
   https://begriffs.com/posts/2021-11-28-practical-parsing.html#using-a-scanner-as-a-library
 */

%option noyywrap nounput noinput
%option noyyalloc noyyrealloc noyyfree

%option prefix="xcc"

/* use the pure parser calling convention */

%option reentrant bison-bridge

%{
#include "xcc_parser.h"

#define YY_EXIT_FAILURE ((void)yyscanner, EXIT_FAILURE)

#include <limits.h>
#include <stdlib.h>
#include <string.h>

#include <xcc.h>

#define YYSTYPE XCCSTYPE

int xccerror(const char *msg);

%}

%%
[a-zA-Z]|\xC2[\xA1-\xFF]|\xC3[\x80-\xBF] {
  yylval->str = strdup(yytext);
  return ID;
}

[[:space:]]  ; /* ignore */

. { return *yytext; }

%%
