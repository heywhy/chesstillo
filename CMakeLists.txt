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

add_library(chesstillo_lib
  STATIC
  src/initializer.cpp
  src/utility.cpp
  src/board.cpp
  src/position.cpp
  src/fen.cpp
  src/fill.cpp
  src/move_gen.cpp
  src/game.cpp
)

target_include_directories(chesstillo_lib PRIVATE include)
target_compile_options(chesstillo_lib PRIVATE ${COMPILE_OPTIONS})
target_link_options(chesstillo_lib PRIVATE -O3)

add_executable(chesstillo_tui tui/theme.cpp tui/utils.cpp tui/main.cpp)
target_include_directories(chesstillo_tui PRIVATE include tui)
target_link_libraries(chesstillo_tui PRIVATE chesstillo_lib)
target_compile_options(chesstillo_tui PRIVATE ${COMPILE_OPTIONS})
target_link_options(chesstillo_tui PRIVATE -O3)

add_executable(chesstillo_perft perft/main.cpp)
target_include_directories(chesstillo_perft PRIVATE include)
target_link_libraries(chesstillo_perft PRIVATE chesstillo_lib)
target_compile_options(chesstillo_perft PRIVATE ${COMPILE_OPTIONS})
target_link_options(chesstillo_perft PRIVATE -O3)

add_executable(chesstillo uci/main.cpp)
target_include_directories(chesstillo PRIVATE include)
target_link_libraries(chesstillo PRIVATE chesstillo_lib)
target_compile_options(chesstillo PRIVATE ${COMPILE_OPTIONS})
target_link_options(chesstillo PRIVATE -O3)

include(FetchContent)

FetchContent_Declare(
  ftxui
  GIT_REPOSITORY https://github.com/ArthurSonzogni/ftxui.git
  GIT_TAG dfb9558eaff417ee7909169d0abd66e6c52c9d55
  GIT_SHALLOW true
)

FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG v1.15.2
  GIT_SHALLOW true
)

FetchContent_Declare(
  magic_bits
  GIT_REPOSITORY https://github.com/goutham/magic-bits.git
  GIT_TAG 3152d1bf63d7ec2ce3b69dc5b2474db7abd419fa
  GIT_SHALLOW true
)

# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(ftxui googletest magic_bits)

add_library(magic_bits INTERFACE)
target_include_directories(magic_bits INTERFACE ${magic_bits_SOURCE_DIR}/include)

target_link_libraries(chesstillo_lib PUBLIC magic_bits)

target_link_libraries(
  chesstillo_tui
  PRIVATE ftxui::screen
  PRIVATE ftxui::dom
  PRIVATE ftxui::component
)

enable_testing()

file(GLOB TEST_SRC LIST_DIRECTORIES false src/*_test.cpp)

add_executable(chesstillo_test ${TEST_SRC})
target_include_directories(chesstillo_test PRIVATE include)
target_link_libraries(
  chesstillo_test
  PRIVATE chesstillo_lib
  PRIVATE GTest::gtest_main
)

include(GoogleTest)

gtest_discover_tests(chesstillo_test)
