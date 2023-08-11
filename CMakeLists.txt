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

add_subdirectory(src)

set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")
set(CMAKE_LINKER_FLAGS_DEBUG "${CMAKE_LINKER_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")

if("${CMAKE_SYSTEM_NAME}" STREQUAL "Emscripten")
  message(STATUS "Compiling on ${CMAKE_SYSTEM_NAME}, so no SAT solver available.")
  set(SRCS_SAT)
  set(SRCS_MAIN
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main_web.cpp
  )
else()
  set(SRCS_SAT
    ${CMAKE_CURRENT_SOURCE_DIR}/src/sat_solver.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/algorithm_knuth_cnf.c
  )
  set(SRCS_MAIN
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main.c
  )

  add_compile_definitions(XCC_SAT_SOLVER_AVAILABLE)
endif()

add_library(xcc STATIC
  ${SRCS}
  ${SRCS_SAT}
)

target_include_directories(xcc PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(xcc PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

add_executable(xccsolve ${SRCS_MAIN})

target_link_libraries(xccsolve xcc)

if("${CMAKE_SYSTEM_NAME}" STREQUAL "Emscripten")
    target_link_options(xcc PRIVATE "-s INITIAL_MEMORY=200MB")
    target_link_options(xcc PRIVATE "--use-preload-cache")
    target_link_options(xcc PRIVATE "--bind")
    target_link_options(xcc PRIVATE "-flto")
    target_link_options(xcc PRIVATE "-Os")
    target_link_options(xcc PRIVATE "-sALLOW_MEMORY_GROWTH")

    target_link_options(xccsolve PRIVATE "-s INITIAL_MEMORY=200MB")
    target_link_options(xccsolve PRIVATE "--use-preload-cache")
    target_link_options(xccsolve PRIVATE "--bind")
    target_link_options(xccsolve PRIVATE "-flto")
    target_link_options(xccsolve PRIVATE "-Os")
    target_link_options(xccsolve PRIVATE "-sALLOW_MEMORY_GROWTH")

    target_compile_options(xccsolve PRIVATE "-flto")
    target_compile_options(xccsolve PRIVATE "-Os")
    target_compile_options(xcc PRIVATE "-flto")
    target_compile_options(xcc PRIVATE "-Os")
endif()

find_package(Catch2)
if(TARGET Catch2::Catch2)
  add_subdirectory(test)
  set_target_properties(tests
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
  )
endif()
