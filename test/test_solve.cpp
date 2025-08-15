#include <algorithm>
#include <vector>

#include <catch_amalgamated.hpp>

#include <miniexact/algorithm.h>
#include <miniexact/algorithm_m.h>
#include <miniexact/algorithm_x.h>
#include <miniexact/parse.h>
#include <miniexact/miniexact.h>

TEST_CASE("solve standard XCC example") {
  const char* str = "<a b c d e f g> c e; a d g; b c f; a d f; b g; d e g;";

  miniexact_algorithm algorithm;
  miniexact_algorithm_x_set(&algorithm);

  miniexact_problem_ptr p(miniexact_parse_problem(&algorithm, str, NULL));
  REQUIRE(p);

  bool has_result = algorithm.compute_next_result(&algorithm, p.get());

  REQUIRE(has_result);

  std::vector<miniexact_link> solution(p->l);
  miniexact_extract_solution_option_indices(p.get(), solution.data());
  std::sort(solution.begin(), solution.end());

  REQUIRE(solution[0] == 1);
  REQUIRE(solution[1] == 4);
  REQUIRE(solution[2] == 5);
}

TEST_CASE("solve small MCC example") {
  const char* str = "<a : 2 b : 1;2> a; a b; b;";
  miniexact_algorithm algorithm;
  miniexact_algorithm_m_set(&algorithm);

  miniexact_problem_ptr p(miniexact_parse_problem(&algorithm, str, NULL));
  REQUIRE(p);

  bool has_result = algorithm.compute_next_result(&algorithm, p.get());

  REQUIRE(has_result);

  std::vector<miniexact_link> solution(p->l);
  miniexact_extract_solution_option_indices(p.get(), solution.data());
  std::sort(solution.begin(), solution.end());

  std::vector<miniexact_link> solution_unsorted(p->l);
  miniexact_extract_solution_option_indices(p.get(), solution_unsorted.data());

  const bool has_duplicates = std::adjacent_find(solution.begin(), solution.end()) != solution.end();

  CAPTURE(solution);
  CAPTURE(solution_unsorted);
  REQUIRE_FALSE(has_duplicates);
}
