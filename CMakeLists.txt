cmake_minimum_required(VERSION 3.29.3)

project(chesstillo)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(FTXUI_QUIET ON)
set(LINK_OPTIONS)

if(NOT MSVC)
  set(COMPILE_OPTIONS -Wall -Wextra -pedantic -fsanitize=undefined -fsanitize=thread)
else()
  set(COMPILE_OPTIONS /std:c++20)
endif()

if (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
  set(LINK_OPTIONS -g -O1 -fsanitize=undefined -fsanitize=thread)
elseif (${CMAKE_BUILD_TYPE} STREQUAL "Release" AND NOT ${MSVC})
  set(COMPILE_OPTIONS -O3)
endif()

list(APPEND CMAKE_MODULE_PATH "${chesstillo_SOURCE_DIR}/cmake")

include(find_googletest)

enable_testing()

add_subdirectory(uci)
add_subdirectory(tui)
add_subdirectory(engine)
