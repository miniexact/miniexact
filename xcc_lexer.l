/* An XCC parser.
 */

/* Following a guide here:
   https://begriffs.com/posts/2021-11-28-practical-parsing.html#using-a-scanner-as-a-library
 */

%option noyywrap nounput noinput
%option prefix="xcc"
%option reentrant bison-bridge
%option header-file="xcc_lexer.h"

%{
#include "xcc_parser.h"

#include <limits.h>
#include <stdlib.h>
#include <string.h>

#include <xcc/xcc.h>
%}

LETTER [^ \n\r\t\f\v\<\>\[\]\:\;]

%%
"<" { return LPRIMLIST; };
">" { return RPRIMLIST; };
"[" { return LSECLIST; };
"]" { return RSECLIST; };
":" { return COLORSEP; };
";" { return ENDOPTION; };

{LETTER}+ {
  yylval->str = strdup(yytext);
  return ID;
}

[[:space:]]  ; /* ignore */
%%
