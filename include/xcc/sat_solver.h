#ifndef XCC_SAT_SOLVER
#define XCC_SAT_SOLVER

#include <stdio.h>
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct xcc_sat_solver {
  int infd[2];
  int outfd[2];
  FILE* infd_handle;
  FILE* outfd_handle;
  pid_t pid;
  unsigned int variables, clauses;
  char* assignments;
} xcc_sat_solver;

void
xcc_sat_solver_init(xcc_sat_solver* solver,
                    unsigned int variables,
                    unsigned int clauses,
                    char* binary,
                    char* const argv[],
                    char* envp[]);

void
xcc_sat_solver_find_and_init(xcc_sat_solver* solver,
                             unsigned int variables,
                             unsigned int clauses);

void
xcc_sat_solver_destroy(xcc_sat_solver* solver);

void
xcc_sat_solver_add(xcc_sat_solver* solver, int l);
void
xcc_sat_solver_unit(xcc_sat_solver* solver, int l);
void
xcc_sat_solver_binary(xcc_sat_solver* solver, int a, int b);
void
xcc_sat_solver_ternary(xcc_sat_solver* solver, int a, int b, int c);

int
xcc_sat_solver_solve(xcc_sat_solver* solver);

#ifdef __cplusplus
}
#endif

#endif
