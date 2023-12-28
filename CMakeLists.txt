cmake_minimum_required(VERSION 3.14)
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

include(CheckGit.cmake)
CheckGitSetup()

add_library(xcc-obj OBJECT
  ${SRCS}
  ${SRCS_SAT}
)
set_property(TARGET xcc-obj PROPERTY POSITION_INDEPENDENT_CODE 1)
target_link_libraries(xcc-obj PUBLIC git_version)
target_include_directories(xcc-obj PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(xcc-obj PUBLIC ${CMAKE_CURRENT_BINARY_DIR})

add_library(xcc-static STATIC $<TARGET_OBJECTS:xcc-obj>)
add_library(xcc SHARED $<TARGET_OBJECTS:xcc-obj>)

target_include_directories(xcc PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(xcc-static PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(xcc PUBLIC git_version)
target_link_libraries(xcc-static PUBLIC git_version)

add_executable(xccsolve ${SRCS_MAIN})

target_link_libraries(xccsolve xcc-static)

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
else()
  # Generate bindings with SWIG, if available
  find_package(SWIG)
  if(SWIG_FOUND)
    cmake_policy(SET CMP0078 NEW)
    cmake_policy(SET CMP0086 NEW)
    include(UseSWIG)

    find_package(Python COMPONENTS Development)
    set_property(SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/swig/pyxcc.i PROPERTY INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/include)
    set_property(SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/swig/pyxcc.i PROPERTY CPLUSPLUS ON)
    set_property(SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/swig/pyxcc.i PROPERTY SWIG_MODULE_NAME pyxcc)
    swig_add_library(pyxcc LANGUAGE Python SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/swig/pyxcc.i)
    target_link_libraries(pyxcc xcc-static Python::Module)
    if(${CMAKE_BUILD_TYPE} STREQUAL "Debug" AND CMAKE_C_COMPILER_ID MATCHES "Clang")
      target_link_libraries(pyxcc -shared-libasan)
      message(STATUS "pyxcc: Using shared libasan, because you are using Clang in a Debug build.")
      message(STATUS "pyxcc: To run, please prepend your interpreter with LD_PRELOAD=$(clang -print-file-name=libclang_rt.asan.so)")
      message(STATUS "pyxcc: To supress memory leak errors, also prepend ASAN_OPTIONS=detect_leaks=0")
    endif()
  else()
    message(STATUS "SWIG not found. Cannot generate bindings to the simple API.")
  endif()
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
