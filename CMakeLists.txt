cmake_minimum_required(VERSION 3.29.3)

project(chesstillo)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(FTXUI_QUIET ON)
option(DEBUG_THREADS "Compile and link builds with ThreadSanitizer." OFF)

if(NOT MSVC)
  list(APPEND COMPILE_OPTIONS -Wall -Wextra -pedantic)
else()
  list(APPEND COMPILE_OPTIONS /std:c++20)
endif()

if (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
  list(APPEND LINK_OPTIONS -g -O1)
elseif (${CMAKE_BUILD_TYPE} STREQUAL "Release" AND NOT ${MSVC})
  list(APPEND COMPILE_OPTIONS -O3)
endif()

if (DEBUG_THREADS AND CMAKE_BUILD_TYPE STREQUAL "Debug")
  message(STATUS "Sanitizers: thread, ub")
  list(APPEND COMPILE_OPTIONS -fsanitize=undefined -fsanitize=thread)
  list(APPEND LINK_OPTIONS -fsanitize=undefined -fsanitize=thread)
endif()

list(APPEND CMAKE_MODULE_PATH "${chesstillo_SOURCE_DIR}/cmake")

include(find_googletest)

enable_testing()

add_subdirectory(uci)
add_subdirectory(tui)
add_subdirectory(engine)
add_subdirectory(nn)

# add_library(chesstillo_lib
#   STATIC
# )
#
# target_include_directories(chesstillo_lib PRIVATE include)
# target_compile_options(chesstillo_lib PRIVATE ${COMPILE_OPTIONS})
# target_link_options(chesstillo_lib PRIVATE -O3)
# target_link_libraries(chesstillo_lib PRIVATE uci)
#
#
# add_executable(chesstillo_perft perft/main.cpp)
# target_include_directories(chesstillo_perft PRIVATE include)
# target_link_libraries(chesstillo_perft PRIVATE chesstillo_lib)
# target_compile_options(chesstillo_perft PRIVATE ${COMPILE_OPTIONS})
# target_link_options(chesstillo_perft PRIVATE -O3)
#
# add_executable(chesstillo uci/main.cpp)
# target_include_directories(chesstillo PRIVATE include)
# target_link_libraries(chesstillo PRIVATE uci)
# target_link_libraries(chesstillo PRIVATE chesstillo_lib)
# target_compile_options(chesstillo PRIVATE ${COMPILE_OPTIONS})
# target_link_options(chesstillo PRIVATE -O3)
#
# FetchContent_Declare(
#   magic_bits
#   GIT_REPOSITORY https://github.com/goutham/magic-bits.git
#   GIT_TAG 3152d1bf63d7ec2ce3b69dc5b2474db7abd419fa
#   GIT_SHALLOW true
# )
#
# FetchContent_Declare(
#   boost
#   GIT_REPOSITORY https://github.com/boostorg/boost.git
#   GIT_TAG boost-1.88.0
#   GIT_SHALLOW true
# )
#
# set(BOOST_INCLUDE_LIBRARIES asio filesystem log process)
# set(BOOST_RUNTIME_LINK static)
#
# FetchContent_MakeAvailable(magic_bits boost)
#
# add_library(magic_bits INTERFACE)
# target_include_directories(
#   magic_bits
#   INTERFACE ${magic_bits_SOURCE_DIR}/include
# )
#
# target_link_libraries(
#   chesstillo_lib
#   PUBLIC magic_bits
#   PUBLIC Boost::asio
#   PUBLIC Boost::log
#   PUBLIC Boost::process
# )
#
#
#
# file(GLOB TEST_SRC LIST_DIRECTORIES false src/*_test.cpp)
#
# add_executable(chesstillo_test ${TEST_SRC})
# target_include_directories(chesstillo_test PRIVATE include)
# target_link_libraries(
#   chesstillo_test
#   PRIVATE uci
#   PRIVATE chesstillo_lib
#   PRIVATE GTest::gtest_main
#   PRIVATE GTest::gmock
# )
#
# gtest_discover_tests(chesstillo_test)
