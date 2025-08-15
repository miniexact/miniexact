set(TEST_SRCS
  test_parse.cpp
  test_solve.cpp
  test_util.cpp
  test_sat_solver.cpp
  test_siftup.cpp

  catch_amalgamated.cpp
)

add_executable(tests ${TEST_SRCS})

target_link_libraries(tests PRIVATE miniexact-static)

target_include_directories(tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

set_property(TARGET tests PROPERTY CXX_STANDARD 20)

target_compile_definitions(tests PRIVATE CATCH_CONFIG_DISABLE_EXCEPTIONS)

include(CTest)
include(Catch)
catch_discover_tests(tests)
