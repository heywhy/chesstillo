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

include(FetchContent)

# UCI

FetchContent_Declare(googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG v1.16.0
  GIT_SHALLOW true
)

# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(googletest)

include(GoogleTest)

add_library(uci
  STATIC
  uci/scanner.cpp
  uci/parser.cpp
  uci/uci.cpp
  uci/exprs/go.cpp
  uci/exprs/position.cpp
  uci/exprs/setoption.cpp
  uci/exprs/register.cpp
  uci/exprs/id.cpp
  uci/exprs/bestmove.cpp
  uci/exprs/copyprotection.cpp
  uci/exprs/registration.cpp
  uci/exprs/info.cpp
  uci/exprs/option.cpp
)

target_include_directories(uci PRIVATE include)

file(GLOB UCI_TEST_SRC LIST_DIRECTORIES false uci/*_test.cpp)

add_executable(uci_test ${UCI_TEST_SRC})
target_include_directories(uci_test PRIVATE include)

target_link_libraries(uci_test
  PRIVATE uci
  PRIVATE GTest::gtest_main
  PRIVATE GTest::gmock
)

gtest_discover_tests(uci_test)

# TUI

FetchContent_Declare(json
  GIT_REPOSITORY https://github.com/nlohmann/json.git
  GIT_TAG v3.12.0
  GIT_SHALLOW true
)

FetchContent_Declare(
  ftxui
  GIT_REPOSITORY https://github.com/ArthurSonzogni/ftxui.git
  GIT_TAG v6.1.9
  GIT_SHALLOW true
)

FetchContent_MakeAvailable(ftxui json)

add_executable(chesstillo_tui tui/theme.cpp tui/main.cpp)

target_include_directories(chesstillo_tui PRIVATE include)

target_link_libraries(chesstillo_tui
  PRIVATE nlohmann_json::nlohmann_json
)

target_compile_options(chesstillo_tui PRIVATE ${COMPILE_OPTIONS})
target_link_options(chesstillo_tui PRIVATE -O3)

target_link_libraries(
  chesstillo_tui
  PRIVATE ftxui::screen
  PRIVATE ftxui::dom
  PRIVATE ftxui::component
)


enable_testing()
