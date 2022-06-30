cmake_minimum_required(VERSION 3.10)
project(xcc)

set(SRCS
  parse.c
  xcc.c
  algorithm.c
  algorithm_x.c
)

set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")
set(CMAKE_LINKER_FLAGS_DEBUG "${CMAKE_LINKER_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")

find_package(FLEX REQUIRED)
find_package(BISON REQUIRED)

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

target_include_directories(xcc PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(xcc PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

add_executable(xccsolve main.c)

target_link_libraries(xccsolve xcc)
