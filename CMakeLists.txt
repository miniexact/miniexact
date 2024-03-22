cmake_minimum_required(VERSION 3.14)
project(miniexact VERSION 1.0)

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

  add_compile_definitions(MINIEXACT_SAT_SOLVER_AVAILABLE)
endif()

add_library(miniexact-obj OBJECT
  ${SRCS}
  ${SRCS_SAT}
)
set_property(TARGET miniexact-obj PROPERTY POSITION_INDEPENDENT_CODE 1)
target_include_directories(miniexact-obj PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(miniexact-obj PUBLIC ${CMAKE_CURRENT_BINARY_DIR})

add_library(miniexact-static STATIC $<TARGET_OBJECTS:miniexact-obj>)

message(STATUS ${CMAKE_C_COMPILER})

if(NOT CMAKE_C_COMPILER MATCHES "cosmo" AND NOT "${CMAKE_SYSTEM_NAME}" STREQUAL "Emscripten")
  add_library(miniexact SHARED $<TARGET_OBJECTS:miniexact-obj>)
  target_include_directories(miniexact PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
endif()

target_include_directories(miniexact-static PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_executable(miniexactsolve ${SRCS_MAIN})
set_target_properties(miniexactsolve PROPERTIES OUTPUT_NAME "miniexact")

target_link_libraries(miniexactsolve miniexact-static)

add_compile_definitions(miniexactsolve PRIVATE MINIEXACT_VERSION="${CMAKE_PROJECT_VERSION}")

if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
  if(MSVC)
    target_compile_options(miniexact-static PRIVATE /W4 /WX)
    target_compile_options(miniexactsolve PRIVATE /W4 /WX)
  else()
    target_compile_options(miniexact-static PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(miniexactsolve PRIVATE -Wall -Wextra -Wpedantic -Werror)
  endif()
endif()

if("${CMAKE_SYSTEM_NAME}" STREQUAL "Emscripten")
  target_link_options(miniexactsolve PRIVATE "-s INITIAL_MEMORY=200MB")
  target_link_options(miniexactsolve PRIVATE "--use-preload-cache")
  target_link_options(miniexactsolve PRIVATE "--bind")
  target_link_options(miniexactsolve PRIVATE "-flto")
  target_link_options(miniexactsolve PRIVATE "-Os")
  target_link_options(miniexactsolve PRIVATE "-sALLOW_MEMORY_GROWTH")

  target_compile_options(miniexactsolve PRIVATE "-flto")
  target_compile_options(miniexactsolve PRIVATE "-Os")
elseif(NOT CMAKE_C_COMPILER MATCHES "cosmo")
  # Generate bindings with SWIG, if available
  find_package(SWIG)
  if(SWIG_FOUND)
    cmake_policy(SET CMP0078 NEW)
    cmake_policy(SET CMP0086 NEW)
    include(UseSWIG)

    find_package(Python COMPONENTS Development)
    set_property(SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/swig/pyminiexact.i PROPERTY INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/include)
    set_property(SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/swig/pyminiexact.i PROPERTY CPLUSPLUS ON)
    set_property(SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/swig/pyminiexact.i PROPERTY SWIG_FLAGS -w389)
    set_property(SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/swig/pyminiexact.i PROPERTY SWIG_MODULE_NAME pyminiexact)
    swig_add_library(pyminiexact LANGUAGE Python SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/swig/pyminiexact.i)
    target_link_libraries(pyminiexact miniexact-static Python::Module)
    set_property(TARGET pyminiexact PROPERTY CXX_STANDARD 20)
    if(${CMAKE_BUILD_TYPE} STREQUAL "Debug" AND CMAKE_C_COMPILER_ID MATCHES "Clang")
      target_link_libraries(pyminiexact -shared-libasan)
      message(STATUS "pyminiexact: Using shared libasan, because you are using Clang in a Debug build.")
      message(STATUS "pyminiexact: To run, please prepend your interpreter with LD_PRELOAD=$(clang -print-file-name=libclang_rt.asan.so)")
      message(STATUS "pyminiexact: To supress memory leak errors, also prepend ASAN_OPTIONS=detect_leaks=0")
    endif()
  else()
    message(STATUS "SWIG not found. Cannot generate bindings to the simple API.")
  endif()
endif()

if(NOT CMAKE_C_COMPILER MATCHES "cosmo" AND NOT "${CMAKE_SYSTEM_NAME}" STREQUAL "Emscripten")
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
endif()
