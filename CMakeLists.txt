cmake_minimum_required(VERSION 3.29.3)

project(chestillo)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(FTXUI_QUIET ON)

add_library(chestillo SHARED src/utility.cpp src/board.cpp src/fen.cpp src/move.cpp src/game.cpp)
target_include_directories(chestillo PRIVATE include)
target_compile_options(chestillo PRIVATE -Wunused-variable)
target_link_options(chestillo PRIVATE -O3)

add_executable(chestillo_gui gui/theme.cpp gui/utils.cpp gui/main.cpp)
target_include_directories(chestillo_gui PRIVATE include gui)
target_link_libraries(chestillo_gui PRIVATE chestillo)
target_compile_options(chestillo_gui PRIVATE -Wall -Wextra)
target_link_options(chestillo_gui PRIVATE -O3)

include(FetchContent)

FetchContent_Declare(
  ftxui
  GIT_REPOSITORY git@github.com:ArthurSonzogni/ftxui.git
  GIT_TAG 307e4eb4b3e0f245b85312e0bd028d748f94b740
  GIT_SHALLOW true
)

FetchContent_Declare(
  googletest
  GIT_REPOSITORY git@github.com:google/googletest.git
  GIT_TAG v1.15.0
  GIT_SHALLOW true
)

# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(ftxui)
FetchContent_MakeAvailable(googletest)

target_link_libraries(
  chestillo_gui
  PRIVATE ftxui::screen
  PRIVATE ftxui::dom
  PRIVATE ftxui::component
)

enable_testing()

file(GLOB TEST_SRC LIST_DIRECTORIES false src/*_test.cpp)

add_executable(chestillo_test ${TEST_SRC})
target_include_directories(chestillo_test PRIVATE include)
target_link_libraries(
  chestillo_test
  PRIVATE chestillo
  PRIVATE GTest::gtest_main
)

include(GoogleTest)

gtest_discover_tests(chestillo_test)
