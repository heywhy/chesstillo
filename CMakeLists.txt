cmake_minimum_required(VERSION 3.29.3)

project(chesstillo)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(FTXUI_QUIET ON)

if(NOT MSVC)
  set(COMPILE_OPTIONS -Wall -Wextra)
else()
  set(COMPILE_OPTIONS /std:c++20)
endif()

list(APPEND CMAKE_MODULE_PATH "${chesstillo_SOURCE_DIR}/cmake")

include(find_googletest)

enable_testing()

add_subdirectory(uci)
