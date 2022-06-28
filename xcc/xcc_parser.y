%define api.pure true
%define api.prefix {xcc}
%define parse.error verbose

%{

%}


%code requires {
#include <stdio.h>
#include <xcc.h>
}

%union
{
    int32_t num;
    char *str;
    xcc_item item;
    xcc_problem *problem;
}

%parse-param {struct xcc_problem **result}

%param {void *scanner}

%code {
    int xccerror(void *foo, char const *msg, const void *s);
    int xcclex(void *lval, const void *s);
}

%token <str> ID
%token <item> ITEM

%destructor { free($$); } <str>
%destructor { xcc_problem_free($$); } <problem>

%type <problem> start
%type <problem> problem

%%

start :
  problem   { *result = $$ = $1; return 0; }
;

problem: '<' "sd" ID ">" { $$ = NULL; };

%%

int xccerror(void *yylval, char const *msg, const void *s)
{
  (void)yylval;
	(void)s;
	return fprintf(stderr, "%s\n", msg);
}
