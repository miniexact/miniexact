#include <algorithm>
#include <array>
#include <iostream>
#include <limits>
#include <numeric>

#include <catch_amalgamated.hpp>

#include <miniexact/siftup.h>

template<typename T>
static void
print_arr(T& arr) {
  for(auto v : arr)
    std::cout << " " << v;
  std::cout << std::endl;
}

void
test_two_arr(uint32_t inserted) {}

TEST_CASE("heap_siftup_uint32") {
  std::array<int32_t, 8> arr, arr2;
  std::fill(arr.begin(), arr.end(), std::numeric_limits<int32_t>::max());

#ifdef __GLIBC__
  heap_siftup(arr.data(), arr.size(), 10);
  heap_siftup(arr.data(), arr.size(), 20);
  REQUIRE(std::is_heap(arr.begin(), arr.end()));
  heap_siftup(arr.data(), arr.size(), 5);
  REQUIRE(std::is_heap(arr.begin(), arr.end()));
  heap_siftup(arr.data(), arr.size(), 5);
  REQUIRE(std::is_heap(arr.begin(), arr.end()));
  heap_siftup(arr.data(), arr.size(), 5);
  REQUIRE(std::is_heap(arr.begin(), arr.end()));
  heap_siftup(arr.data(), arr.size(), 2);
  REQUIRE(std::is_heap(arr.begin(), arr.end()));
  heap_siftup(arr.data(), arr.size(), 3);
  REQUIRE(std::is_heap(arr.begin(), arr.end()));
  heap_siftup(arr.data(), arr.size(), 8);
  REQUIRE(std::is_heap(arr.begin(), arr.end()));
  heap_siftup(arr.data(), arr.size(), 5);
  REQUIRE(std::is_heap(arr.begin(), arr.end()));
  heap_siftup(arr.data(), arr.size(), 5);
  REQUIRE(std::is_heap(arr.begin(), arr.end()));
  heap_siftup(arr.data(), arr.size(), 8);
  REQUIRE(std::is_heap(arr.begin(), arr.end()));
#endif
}
