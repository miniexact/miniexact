#include "algorithm.hpp"
#include "delta-debug-problem.hpp"
#include "parser.hpp"
#include "problem.hpp"
#include "wordpuzzle.hpp"
#include <algorithm>
#include <iterator>
#include <variant>

#define DOCTEST_CONFIG_NO_POSIX_SIGNALS
#define DOCTEST_CONFIG_IMPLEMENT
#define DOCTEST_CONFIG_NO_UNPREFIXED_OPTIONS
#include "doctest.h"

#define IGN 0

namespace dancing_links {

auto
produce_vectors_for_example_49() {
  using H = HNode<char>;
  using N = Node<char>;
  auto pair = std::pair<HNodeVector<char>, NodeVector<char>>(
    HNodeVector<char>{ H(' ', 3, 1),
                       H('p', 0, 2),
                       H('q', 1, 3),
                       H('r', 2, 0),
                       H('x', 6, 5),
                       H('y', 4, 6),
                       H(' ', 5, 4) },

    NodeVector<char>{ // Row 2
                      N(IGN, IGN, IGN),
                      N(3, 17, 7),
                      N(2, 20, 8),
                      N(2, 23, 13),
                      N(4, 21, 9),
                      N(3, 24, 10),
                      N(0, IGN, 10),
                      // Row 3
                      N(1, 1, 12, 0),
                      N(2, 2, 20, 0),
                      N(4, 4, 14, 0),
                      N(5, 5, 15, 'A'),
                      N(-1, 7, 15, 0),
                      N(1, 7, 17, 0),
                      N(3, 3, 23, 0),
                      // Row 4
                      N(4, 9, 18, 'A'),
                      N(5, 10, 24, 0),
                      N(-2, 12, 18, 0),
                      N(1, 12, 1, 0),
                      N(4, 14, 21, 'B'),
                      N(-3, 17, 21, 0),
                      N(2, 8, 2, 0),
                      // Row 5
                      N(4, 18, 4, 'A'),
                      N(-4, 20, 24, 0),
                      N(3, 13, 3, 0),
                      N(5, 15, 5, 'B'),
                      N(-5, 23, IGN, 0) });

  // Check sizes compared to the book indexes.
  CHECK(pair.first.size() == 7);
  CHECK(pair.second.size() == 26);
  return pair;
}

TEST_CASE("Algorithm C example problem from page 87") {
  auto vecs = produce_vectors_for_example_49();
  auto& hnvec = vecs.first;
  auto& nvec = vecs.second;

  AlgorithmC<HNodeVector<char>, NodeVector<char>> xcc(hnvec, nvec);

  // Printing solutions during testing:
  // while(bool solution_available = xcc.compute_next_solution()) {
  //  cout << "Solution: " << endl;
  //  std::for_each(
  //    s.begin(), s.end(), [&nvec](auto n) { cout << nvec[n].TOP << endl; });
  //}

  bool solution_available = xcc.compute_next_solution();

  REQUIRE(solution_available);

  const auto& s = xcc.current_selected_options();
  REQUIRE(s.size() == 2);
  REQUIRE((s[0] == 4 || s[0] == 2));
  REQUIRE((s[1] == 2 || s[1] == 4));

  solution_available = xcc.compute_next_solution();

  REQUIRE(!solution_available);
}

TEST_CASE("Parse example problem from page 87") {
  std::string_view problemStr =
    "<p q r> [x y] p q x y:A; p r x:A y; p x:B; q x:A; r y:B .";
  auto problemOpt = parse_string_mapped_int32(problemStr);

  REQUIRE(problemOpt);

  auto problem = problemOpt.value();

  REQUIRE(problem.getPrimaryItemCount() == 3);
  REQUIRE(problem.getSecondaryItemCount() == 2);

  auto staticallyDefinedVectorPair = produce_vectors_for_example_49();
  auto& shna = staticallyDefinedVectorPair.first;
  auto& sna = staticallyDefinedVectorPair.second;

  for(size_t i = 0; i < shna.size(); ++i) {
    CAPTURE(i);

    REQUIRE(problem.hna[i].LLINK == shna[i].LLINK);
    REQUIRE(problem.hna[i].RLINK == shna[i].RLINK);
  }
  for(size_t i = 0; i < sna.size(); ++i) {
    CAPTURE(i);

    REQUIRE(problem.na[i].TOP == sna[i].TOP);
    REQUIRE(problem.na[i].ULINK == sna[i].ULINK);
    REQUIRE(problem.na[i].DLINK == sna[i].DLINK);
  }

  std::vector<std::vector<int>> optionsVec;
  for(auto o : problem) {
    std::vector<int> items;
    for(auto i : o) {
      if(auto pi = std::get_if<PrimaryItem<int>>(&i)) {
        items.push_back(pi->item);
      } else if(auto ci = std::get_if<ColoredItem<int, int>>(&i)) {
        items.push_back(ci->item);
      } else {
        REQUIRE(false);
      }
    }
    optionsVec.push_back(items);
  }

  CHECK(optionsVec[0][0] == 1);
  CHECK(optionsVec[0][1] == 2);
  CHECK(optionsVec[0][2] == 4);
  CHECK(optionsVec[0][3] == 5);
  CHECK(optionsVec[1][0] == 1);
  CHECK(optionsVec[1][1] == 3);
}

TEST_CASE("Delta Debug a Problem to Minify Required Options for SAT") {
  std::string_view problemStr = "<a b c> [ d ] a; b; c; a b; a b d;";
  auto problemOpt = parse_string_mapped_int32(problemStr);

  REQUIRE(problemOpt);

  auto& problem = *problemOpt;

  DeltaDebugProblem dd(problem);

  auto reducedOpt = dd.keep_sat_while_removing_options();

  auto& reduced = *reducedOpt;

  std::cout << "Minified: " << std::endl;
  reduced.printMapped(std::cout);
}
}
