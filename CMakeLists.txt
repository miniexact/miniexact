cmake_minimum_required(VERSION 3.18)
project(miniexact VERSION 1.4)

# Set a default build type if none was specified
set(default_build_type "Release")

set(CMAKE_CXX_STANDARD 20)

set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH}" "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Setting build type to '${default_build_type}' as none was specified.")
  set(CMAKE_BUILD_TYPE "${default_build_type}" CACHE
      STRING "Choose the type of build." FORCE)
  # Set the possible values of build type for cmake-gui
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
    "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

include(CheckSymbolExists)
check_symbol_exists(strlcpy "string.h" HAVE_STRLCPY)
if(HAVE_STRLCPY)
  add_compile_definitions(HAVE_STRLCPY)
endif()

file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/license.h" "")
file(READ "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE.md" LICENSE_TEXT HEX)
string(REGEX REPLACE "([0-9a-f][0-9a-f])" "0x\\1," LICENSE_TEXT ${LICENSE_TEXT})
file(APPEND "${CMAKE_CURRENT_BINARY_DIR}/license.h" "const unsigned char miniexact_license[] = {${LICENSE_TEXT}0x00};")
include_directories("${CMAKE_CURRENT_BINARY_DIR}")

add_subdirectory(src)

if(NOT CMAKE_SYSTEM_NAME MATCHES "OpenBSD")
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")
    set(CMAKE_LINKER_FLAGS_DEBUG "${CMAKE_LINKER_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")
endif()

if("${CMAKE_SYSTEM_NAME}" STREQUAL "Emscripten")
  message(STATUS "Compiling on ${CMAKE_SYSTEM_NAME}, so no SAT solver available.")
  set(SRCS_SAT)
  set(SRCS_MAIN
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main_web.cpp
  )
elseif(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
  message(STATUS "Compiling on ${CMAKE_SYSTEM_NAME}, so no SAT solver available.")
  set(SRCS_SAT)
  set(SRCS_MAIN
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/getopt_port.c
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

if(SKBUILD)
  add_library(miniexact-obj OBJECT
    ${SRCS}
    ${SRCS_SAT}
  )
  set_property(TARGET miniexact-obj PROPERTY POSITION_INDEPENDENT_CODE 1)
  target_include_directories(miniexact-obj PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
  target_include_directories(miniexact-obj PUBLIC ${CMAKE_CURRENT_BINARY_DIR})

  add_library(miniexact-static STATIC $<TARGET_OBJECTS:miniexact-obj>)

  target_include_directories(miniexact-static PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

  add_executable(miniexactsolve ${SRCS_MAIN})
  target_link_libraries(miniexactsolve miniexact-static)
  install(TARGETS miniexactsolve DESTINATION "${SKBUILD_SCRIPTS_DIR}")
elseif(NOT ${CMAKE_C_COMPILER} MATCHES "cosmo")
  add_library(miniexact-obj OBJECT
    ${SRCS}
    ${SRCS_SAT}
  )
  set_property(TARGET miniexact-obj PROPERTY POSITION_INDEPENDENT_CODE 1)
  target_include_directories(miniexact-obj PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
  target_include_directories(miniexact-obj PUBLIC ${CMAKE_CURRENT_BINARY_DIR})

  add_library(miniexact-static STATIC $<TARGET_OBJECTS:miniexact-obj>)

  if(NOT "${CMAKE_C_COMPILER}" MATCHES "cosmo" AND NOT "${CMAKE_SYSTEM_NAME}" STREQUAL "Emscripten")
    add_library(miniexact SHARED $<TARGET_OBJECTS:miniexact-obj>)
    target_include_directories(miniexact PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
  endif()

  target_include_directories(miniexact-static PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

  add_executable(miniexactsolve ${SRCS_MAIN})
  target_link_libraries(miniexactsolve miniexact-static)
else()
  add_executable(miniexactsolve ${SRCS_MAIN} ${SRCS} ${SRCS_SAT})
  target_include_directories(miniexactsolve PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
  target_include_directories(miniexactsolve PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
endif()

if(TARGET miniexactsolve)
  set_target_properties(miniexactsolve PROPERTIES OUTPUT_NAME "miniexact")
  add_compile_definitions(miniexactsolve PRIVATE MINIEXACT_VERSION="${CMAKE_PROJECT_VERSION}")
endif()

if("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
  if(MSVC)
    target_compile_options(miniexact-static PRIVATE /W4 /WX)
    if(TARGET miniexactsolve)
      target_compile_options(miniexactsolve PRIVATE /W4 /WX)
    endif()
  else()
    target_compile_options(miniexact-static PRIVATE -Wall -Wextra -Wpedantic -Werror)
    if(TARGET miniexactsolve)
      target_compile_options(miniexactsolve PRIVATE -Wall -Wextra -Wpedantic -Werror)
    endif()
  endif()
endif()

if("${CMAKE_SYSTEM_NAME}" STREQUAL "Emscripten" AND TARGET miniexactsolve)
  target_link_options(miniexactsolve PRIVATE "-s INITIAL_MEMORY=200MB")
  target_link_options(miniexactsolve PRIVATE "--use-preload-cache")
  target_link_options(miniexactsolve PRIVATE "--bind")
  target_link_options(miniexactsolve PRIVATE "-flto")
  target_link_options(miniexactsolve PRIVATE "-Os")
  target_link_options(miniexactsolve PRIVATE "-sALLOW_MEMORY_GROWTH")

  target_compile_options(miniexactsolve PRIVATE "-flto")
  target_compile_options(miniexactsolve PRIVATE "-Os")
endif()

if(NOT CMAKE_C_COMPILER MATCHES "cosmo" AND NOT ("${CMAKE_SYSTEM_NAME}" STREQUAL "Emscripten" AND NOT SKBUILD))
  # Generate bindings with SWIG, if available
  find_package(SWIG)
  if(SWIG_FOUND)
    cmake_policy(SET CMP0078 NEW)
    cmake_policy(SET CMP0086 NEW)
    include(UseSWIG)

    find_package(Python COMPONENTS Interpreter Development.Module REQUIRED)
    set_property(SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/swig/miniexact.i PROPERTY INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/include)
    set_property(SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/swig/miniexact.i PROPERTY CPLUSPLUS ON)
    set_property(SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/swig/miniexact.i PROPERTY SWIG_FLAGS -w389)
    set_property(SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/swig/miniexact.i PROPERTY SWIG_MODULE_NAME miniexact)
    swig_add_library(pyminiexact LANGUAGE Python SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/swig/miniexact.i)
    target_link_libraries(pyminiexact miniexact-static Python::Module)
    set_property(TARGET pyminiexact PROPERTY CXX_STANDARD 20)
    if("${CMAKE_BUILD_TYPE}" STREQUAL "Debug" AND CMAKE_C_COMPILER_ID MATCHES "Clang" AND NOT CMAKE_SYSTEM_NAME MATCHES "OpenBSD")
      target_link_libraries(pyminiexact -shared-libasan)
      message(STATUS "pyminiexact: Using shared libasan, because you are using Clang in a Debug build.")
      message(STATUS "pyminiexact: To run, please prepend your interpreter with LD_PRELOAD=$(clang -print-file-name=libclang_rt.asan.so)")
      message(STATUS "pyminiexact: To supress memory leak errors, also prepend ASAN_OPTIONS=detect_leaks=0")
    endif()

    install(TARGETS pyminiexact LIBRARY DESTINATION miniexact)
    install(FILES "${CMAKE_CURRENT_BINARY_DIR}/miniexact.py"
      DESTINATION miniexact
      RENAME "__init__.py")
  else()
    message(STATUS "SWIG not found. Cannot generate bindings to the simple API.")
  endif()
endif()

if(NOT CMAKE_C_COMPILER MATCHES "cosmo" AND NOT "${CMAKE_SYSTEM_NAME}" STREQUAL "Emscripten")
  add_subdirectory(test)
  set_target_properties(tests
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
  )

  # Ensure that there is a "test" target.
  include(CTest)
  add_test(tests tests)
else()
  include(CTest)
endif()
