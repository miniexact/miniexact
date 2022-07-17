#ifndef XCC_PARSE_H
#define XCC_PARSE_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct xcc_problem xcc_problem;
typedef struct xcc_algorithm xcc_algorithm;

xcc_problem*
xcc_parse_problem(xcc_algorithm* a, const char* str);

#ifdef __cplusplus
}
#endif

#endif
