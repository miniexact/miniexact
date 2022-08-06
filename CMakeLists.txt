cmake_minimum_required(VERSION 3.10)
project(xcc)

# Set a default build type if none was specified
set(default_build_type "Release")

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Setting build type to '${default_build_type}' as none was specified.")
  set(CMAKE_BUILD_TYPE "${default_build_type}" CACHE
      STRING "Choose the type of build." FORCE)
  # Set the possible values of build type for cmake-gui
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
    "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

set(SRCS
  parse.c
  xcc.c
  algorithm.c
  algorithm_x.c
  algorithm_c.c
  algorithm_knuth_cnf.c
  log.c
)

set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")
set(CMAKE_LINKER_FLAGS_DEBUG "${CMAKE_LINKER_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")

find_package(FLEX REQUIRED)
find_package(BISON REQUIRED)

add_subdirectory(third_party)

flex_target(xcc_lexer
  "${CMAKE_CURRENT_SOURCE_DIR}/xcc_lexer.l"
  "${CMAKE_CURRENT_BINARY_DIR}/xcc_lexer.c")

bison_target(xcc_parser
  "${CMAKE_CURRENT_SOURCE_DIR}/xcc_parser.y"
  "${CMAKE_CURRENT_BINARY_DIR}/xcc_parser.c")

add_flex_bison_dependency(xcc_lexer xcc_parser)

add_library(xcc STATIC
  ${SRCS}
  "${CMAKE_CURRENT_BINARY_DIR}/xcc_lexer.c"
  "${CMAKE_CURRENT_BINARY_DIR}/xcc_parser.c"
)

target_link_libraries(xcc libcadical)

target_include_directories(xcc PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(xcc PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

add_executable(xccsolve main.c)

target_link_libraries(xccsolve xcc)

add_subdirectory(test)
set_target_properties(tests
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)
