#include <catch2/catch_test_macros.hpp>

#include <xcc/util.h>

TEST_CASE("xcc_sign") {
  REQUIRE(xcc_sign(1) == true);
  REQUIRE(xcc_sign(0) == true);
  REQUIRE(xcc_sign(-1) == false);
}
