cmake_minimum_required(VERSION 3.29.3)

project(chesstillo)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

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
