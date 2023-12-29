#include <catch2/catch_test_macros.hpp>

#include <miniexact/util.h>

TEST_CASE("miniexact_sign") {
  REQUIRE(miniexact_sign(1) == true);
  REQUIRE(miniexact_sign(0) == true);
  REQUIRE(miniexact_sign(-1) == false);
}
