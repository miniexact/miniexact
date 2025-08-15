#include <algorithm>
#include <vector>
#include <cstring>

#include <catch_amalgamated.hpp>

#ifdef MINIEXACT_SAT_SOLVER_AVAILABLE

#include <miniexact/sat_solver.h>

TEST_CASE("Gather an UNSAT result from a SAT Solver") {
  miniexact_sat_solver solver;
  std::memset(&solver, 0, sizeof(solver));
  char* arr1[] = { (char*)"-c", (char*)"exit 20", NULL };
  miniexact_sat_solver_init(&solver, 2, 2, (char*)"/bin/sh", arr1, NULL);
  miniexact_sat_solver_unit(&solver, 1);
  miniexact_sat_solver_unit(&solver, 2);
  int status = miniexact_sat_solver_solve(&solver);
  REQUIRE(status == 20);

  // Second time, in order to check if re-entrant solving works
  char* arr2[] = { (char*)"-c", (char*)"exit 20", NULL };
  miniexact_sat_solver_init(&solver, 2, 2, (char*)"/bin/sh", arr2, NULL);
  miniexact_sat_solver_unit(&solver, -1);
  miniexact_sat_solver_unit(&solver, -2);
  status = miniexact_sat_solver_solve(&solver);
  REQUIRE(status == 20);

  miniexact_sat_solver_destroy(&solver);
}

TEST_CASE("Gather a SAT result from a SAT Solver") {
  miniexact_sat_solver solver;
  std::memset(&solver, 0, sizeof(solver));
  char* arr[] = { (char*)"-c", (char*)"echo \"v 1 -2\"; exit 10", NULL };
  miniexact_sat_solver_init(&solver, 2, 2, (char*)"/bin/sh", arr, NULL);
  miniexact_sat_solver_unit(&solver, 1);
  miniexact_sat_solver_unit(&solver, 2);
  int status = miniexact_sat_solver_solve(&solver);
  REQUIRE(status == 10);

  REQUIRE(solver.assignments[1] == true);
  REQUIRE(solver.assignments[2] == false);

  miniexact_sat_solver_destroy(&solver);
}

#endif
