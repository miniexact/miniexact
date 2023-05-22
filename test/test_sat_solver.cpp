#include <algorithm>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include <xcc/sat_solver.h>

TEST_CASE("Gather an UNSAT result from a SAT Solver") {
  xcc_sat_solver solver;
  std::memset(&solver, 0, sizeof(solver));
  xcc_sat_solver_init(&solver,
                      2,
                      2,
                      (char*)"/bin/sh",
                      (char*[]){ (char*)"-c", (char*)"exit 20", NULL },
                      NULL);
  xcc_sat_solver_unit(&solver, 1);
  xcc_sat_solver_unit(&solver, 2);
  int status = xcc_sat_solver_solve(&solver);
  REQUIRE(status == 20);

  // Second time, in order to check if re-entrant solving works
  xcc_sat_solver_init(&solver,
                      2,
                      2,
                      (char*)"/bin/sh",
                      (char*[]){ (char*)"-c", (char*)"exit 20", NULL },
                      NULL);
  xcc_sat_solver_unit(&solver, -1);
  xcc_sat_solver_unit(&solver, -2);
  status = xcc_sat_solver_solve(&solver);
  REQUIRE(status == 20);

  xcc_sat_solver_destroy(&solver);
}

TEST_CASE("Gather a SAT result from a SAT Solver") {
  xcc_sat_solver solver;
  std::memset(&solver, 0, sizeof(solver));
  xcc_sat_solver_init(
    &solver,
    2,
    2,
    (char*)"/bin/sh",
    (char*[]){ (char*)"-c", (char*)"echo \"v 1 -2\"; exit 10", NULL },
    NULL);
  xcc_sat_solver_unit(&solver, 1);
  xcc_sat_solver_unit(&solver, 2);
  int status = xcc_sat_solver_solve(&solver);
  REQUIRE(status == 10);

  REQUIRE(solver.assignments[1] == true);
  REQUIRE(solver.assignments[2] == false);

  xcc_sat_solver_destroy(&solver);
}
