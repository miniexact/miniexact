#include <algorithm>
#include <numeric>
#include <array>
#include <iostream>

#include <catch2/catch_test_macros.hpp>

#include <miniexact/siftup.h>

template<typename T>
static void
print_arr(T& arr) {
  for(auto v : arr)
    std::cout << " " << v;
  std::cout << std::endl;
}

void test_two_arr(uint32_t inserted) {
  
}

TEST_CASE("heap_siftup_uint32") {
  std::array<uint32_t, 8> arr, arr2;
  std::iota(arr.begin() + 1, arr.end(), 10);
  std::make_heap(arr.begin() + 1, arr.end());
  
  print_arr(arr);
  heap_siftup(arr.data(), arr.size() - 2, 20);
  heap_siftup(arr.data(), arr.size() - 2, 10);
  heap_siftup(arr.data(), arr.size() - 2, 12);
  heap_siftup(arr.data(), arr.size() - 2, 12);
  print_arr(arr);

  // REQUIRE(false);
}
