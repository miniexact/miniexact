%define api.pure full
%locations
%define api.prefix {xcc}
%define parse.error verbose



%{

%}


%code top {
/* XOPEN for strdup */
#define _XOPEN_SOURCE 600
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Bison versions 3.7.5 and above provide the YYNOMEM
    macro to allow our actions to signal the unlikely
    event that they couldn't allocate memory. Thanks
    to the Bison team for adding this feature at my
    request. :) YYNOMEM causes yyparse() to return 2.

    The following conditional define allows us to use
    the functionality in earlier versions too. */

#ifndef YYNOMEM
#define YYNOMEM goto yyexhaustedlab
#endif
}


%code requires {
#include <xcc.h>

#ifndef YYSTYPE
#define YYSTYPE XCCSTYPE
#endif

#ifndef YYLTYPE
#define YYLTYPE XCCLTYPE
#endif

typedef void *yyscan_t;
int xcclex_init(yyscan_t*);
int xcclex_destroy(yyscan_t);
 void xcc_yy_parse_string(const char* str, xcc_algorithm *a, xcc_problem** p);
}

%union
{
    int32_t num;
    char *str;
    xcc_item item;
    xcc_problem *problem;
}

%parse-param {struct xcc_algorithm *algorithm}
%parse-param {struct xcc_problem **result}
%parse-param {void *scanner}

%code {

#include <xcc_lexer.h>

YY_BUFFER_STATE xcc_scan_string ( const char *yy_str , yyscan_t yyscanner );
void xcc_switch_to_buffer ( YY_BUFFER_STATE new_buffer , yyscan_t yyscanner );

 void xcc_yy_parse_string(const char* str, xcc_algorithm *a, xcc_problem** p)
{
    yyscan_t scanner;
    xcclex_init(&scanner);
    xccparse(a, p, scanner);
    xcclex_destroy(scanner);
}
}

%code provides {
    //#define YY_DECL							\
    //int yylex(YYSTYPE* yylval, YYLTYPE* yylloc, xcc_algorithm* a, xcc_problem **result, yyscan_t *yyscanner)
    //YY_DECL;

   int xccerror(XCCLTYPE *loc, void *algorithm,
		 void *problem,
		 const void* scanner,
		 char const *msg);
}

%token <num> NUM
%token <str> ID
%token <item> ITEM

%destructor { free($$); } <str>

%type	start
%type	problem
%type primary_items
%type primary_item
%type secondary_items
%type secondary_item
%type options
%type option

%%

start :
  problem
;

problem:	{ *result = xcc_problem_allocate(); }
		'<' primary_items '>'
		'[' secondary_items ']'
		options
	;

primary_items:	primary_item
	|	primary_item primary_items
	;

primary_item: 	ID { algorithm->define_primary_item(algorithm, *result, $1); }
        |	ID ':' '[' NUM ';' NUM ']'
		{ algorithm->define_primary_item_with_range(algorithm, *result, $1, $4, $6); }
	;

secondary_items:
		secondary_item
	|	secondary_item secondary_items
	;

secondary_item:	ID { algorithm->define_secondary_item(algorithm, *result, $1); }
	;

options:	option ';'
	|	options option
	;

option:		ID { algorithm->add_option(algorithm,
					   *result,
					   xcc_option_from_ident(*result, $1));
		}
	|	ID ':' NUM { algorithm->add_option_with_color(algorithm,
							      *result,
							      xcc_option_from_ident(*result, $1),
							      $3);
		}
	;

%%

int xccerror(XCCLTYPE *loc, void *algorithm,
		void *problem,
		const void* scanner,
		char const *msg)
{
    (void)algorithm;
    (void)problem;
    (void)scanner;
    return fprintf(stderr, "XCC: %s\n", msg);
}
