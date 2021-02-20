cmake_minimum_required(VERSION 3.8)
project(DancingLinks)

set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)

find_package(Boost COMPONENTS program_options filesystem REQUIRED)

add_library(libdancinglinks STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/wordpuzzle.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/parser.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/problem.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/algorithm.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/emscripten.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/util.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/delta-debug-problem.cpp
)

set_target_properties(libdancinglinks PROPERTIES OUTPUT_NAME "dancinglinks")

target_link_libraries(libdancinglinks Boost::filesystem)
target_compile_features(libdancinglinks PUBLIC cxx_std_17)

add_executable(dancing-links
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/test.cpp
)

target_link_libraries(dancing-links libdancinglinks)

target_link_libraries(dancing-links Boost::program_options)

# Address Sanitizer for Debug Builds
if(DEBUG_ENABLE_ADDRESS_SANITIZER)
    set(CMAKE_BUILD_TYPE Debug)
    set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -DBOOST_USE_ASAN")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address  -fsanitize=undefined -fsanitize=address -fno-sanitize-recover=undefined -fno-sanitize=vptr")
    set(CMAKE_LINKER_FLAGS_DEBUG "${CMAKE_LINKER_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address  -fsanitize=undefined -fsanitize=address -fno-sanitize-recover=undefined -fno-sanitize=vptr")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fno-omit-frame-pointer -fsanitize=address  -fsanitize=undefined -fsanitize=address -fno-sanitize-recover=undefined -fno-sanitize=vptr")
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address  -fsanitize=undefined -fsanitize=address -fno-sanitize-recover=undefined -fno-sanitize=vptr")
endif()

# Thread Sanitizer for Debug Builds
if(DEBUG_ENABLE_THREAD_SANITIZER)
    set(CMAKE_BUILD_TYPE Debug)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=thread")
    set(CMAKE_LINKER_FLAGS_DEBUG "${CMAKE_LINKER_FLAGS_DEBUG} -fsanitize=thread")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=thread")
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -fsanitize=thread")
endif()
