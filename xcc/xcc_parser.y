%define api.pure true
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

typedef void *yyscan_t;
int xcclex_init(yyscan_t*);
int xcclex_destroy(yyscan_t);
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

%param {void *scanner}

%code {
    int xccerror(void *algorithm,
		 void *problem,
		 const void* scanner,
		 char const *msg);

#define YYSTYPE XCCSTYPE
#include <xcc_lexer.h>
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

int xccerror(void *algorithm,
		void *problem,
		const void* scanner,
		char const *msg)
{
    (void)algorithm;
    (void)problem;
    (void)scanner;
    return fprintf(stderr, "XCC: %s\n", msg);
}

void xcc_yy_parse_string(const char* str, yyscan_t *scanner)
{
    xcc_switch_to_buffer(xcc_scan_string(str, scanner), scanner);
}
