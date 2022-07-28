#include <catch2/catch_test_macros.hpp>

#include <algorithm.h>
#include <algorithm_x.h>
#include <algorithm_c.h>
#include <parse.h>
#include <xcc.h>

#include <ops.h>

TEST_CASE("parse standard XCC example") {
  const char* str = "<a b c d e f g> c e; a d g; b c f; a d f; b g; d e g;";

  xcc_algorithm algorithm;
  xcc_algoritihm_x_set(&algorithm);

  xcc_problem_ptr p(xcc_parse_problem(&algorithm, str));
  REQUIRE(p);

  // Reference solution from page 66 of Dancing Links (Table 1)

  REQUIRE(NAME(0) == NULL);
  REQUIRE(LLINK(0) == 7);
  REQUIRE(RLINK(0) == 1);

#define REQH(I, N, L, R)              \
  REQUIRE(std::string(NAME(I)) == N); \
  REQUIRE(LLINK(I) == L);             \
  REQUIRE(RLINK(I) == R)

  REQH(1, "a", 0, 2);
  REQH(2, "b", 1, 3);
  REQH(3, "c", 2, 4);
  REQH(4, "d", 3, 5);
  REQH(5, "e", 4, 6);
  REQH(6, "f", 5, 7);
  REQH(7, "g", 6, 0);

#undef REQH
#define REQN(I, L, U, D)  \
  REQUIRE(LEN(I) == L);   \
  REQUIRE(ULINK(I) == U); \
  REQUIRE(DLINK(I) == D)

  REQN(0, 0, 0, 0);
  REQN(1, 2, 20, 12);
  REQN(2, 2, 24, 16);
  REQN(3, 2, 17, 9);
  REQN(4, 3, 27, 13);
  REQN(5, 2, 28, 10);
  REQN(6, 2, 22, 18);
  REQN(7, 3, 29, 14);

  REQN(8, 0, 0, 10);
  REQN(9, 3, 3, 17);
  REQN(10, 5, 5, 28);
  REQN(11, -1, 9, 14);
  REQN(12, 1, 1, 20);
  REQN(13, 4, 4, 21);
  REQN(14, 7, 7, 25);
  REQN(15, -2, 12, 18);

  REQN(16, 2, 2, 24);
  REQN(17, 3, 9, 3);
  REQN(18, 6, 6, 22);
  REQN(19, -3, 16, 22);
  REQN(20, 1, 12, 1);
  REQN(21, 4, 13, 27);
  REQN(22, 6, 18, 6);
  REQN(23, -4, 20, 25);

  REQN(24, 2, 16, 2);
  REQN(25, 7, 14, 29);
  REQN(26, -5, 24, 29);
  REQN(27, 4, 21, 4);
  REQN(28, 5, 10, 5);
  REQN(29, 7, 25, 7);
  REQN(30, -6, 27, 0);

#undef REQN

  // Compare problem layout against reference in book

  // if(p) {
  //   bool has_solution = algorithm.compute_next_result(&algorithm, p);
  //   if(has_solution)
  //     xcc_print_problem_solution(p);
  //   else
  //     printf("No solution found!\n");
  //   xcc_problem_free(p);
  // }
}

TEST_CASE("parse standard colored XCC example") {
  const char* str = "< p q r > [ x y ] p q x y:1; p r x:1 y; p x:2; q x:1; r y:2;";

  xcc_algorithm algorithm;
  xcc_algoritihm_c_set(&algorithm);

  xcc_problem_ptr p(xcc_parse_problem(&algorithm, str));
  REQUIRE(p);

  // Reference solution from page 66 of Dancing Links (Table 1)

  REQUIRE(NAME(0) == NULL);
  REQUIRE(LLINK(0) == 3);
  REQUIRE(RLINK(0) == 1);

#define REQH(I, N, L, R)              \
  REQUIRE(std::string(NAME(I)) == N); \
  REQUIRE(LLINK(I) == L);             \
  REQUIRE(RLINK(I) == R)

  REQH(1, "p", 0, 2);
  REQH(2, "q", 1, 3);
  REQH(3, "r", 2, 6);
  REQH(4, "x", 6, 5);
  REQH(5, "y", 4, 6);

#undef REQH
#define REQN(I, L, U, D)  \
  REQUIRE(LEN(I) == L);   \
  REQUIRE(ULINK(I) == U); \
  REQUIRE(DLINK(I) == D)

  REQN(0, 0, 0, 0);
  REQN(1, 2, 20, 12);
  REQN(2, 2, 24, 16);
  REQN(3, 2, 17, 9);
  REQN(4, 3, 27, 13);
  REQN(5, 2, 28, 10);
  REQN(6, 2, 22, 18);
  REQN(7, 3, 29, 14);

  REQN(8, 0, 0, 10);
  REQN(9, 3, 3, 17);
  REQN(10, 5, 5, 28);
  REQN(11, -1, 9, 14);
  REQN(12, 1, 1, 20);
  REQN(13, 4, 4, 21);
  REQN(14, 7, 7, 25);
  REQN(15, -2, 12, 18);

  REQN(16, 2, 2, 24);
  REQN(17, 3, 9, 3);
  REQN(18, 6, 6, 22);
  REQN(19, -3, 16, 22);
  REQN(20, 1, 12, 1);
  REQN(21, 4, 13, 27);
  REQN(22, 6, 18, 6);
  REQN(23, -4, 20, 25);

  REQN(24, 2, 16, 2);
  REQN(25, 7, 14, 29);
  REQN(26, -5, 24, 29);
  REQN(27, 4, 21, 4);
  REQN(28, 5, 10, 5);
  REQN(29, 7, 25, 7);
  REQN(30, -6, 27, 0);

#undef REQN

  // Compare problem layout against reference in book

  // if(p) {
  //   bool has_solution = algorithm.compute_next_result(&algorithm, p);
  //   if(has_solution)
  //     xcc_print_problem_solution(p);
  //   else
  //     printf("No solution found!\n");
  //   xcc_problem_free(p);
  // }
}
