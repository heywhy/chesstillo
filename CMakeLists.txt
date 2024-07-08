cmake_minimum_required(VERSION 3.29.3)

project(chestillo)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable(chestillo src/board.cpp src/fen.cpp src/move.cpp src/main.cpp)
target_compile_options(chestillo PRIVATE -Wunused-variable)
target_link_options(chestillo PRIVATE -O3)

include(FetchContent)

FetchContent_Declare(
  googletest
  GIT_REPOSITORY git@github.com:google/googletest.git
  GIT_TAG 34ad51b3dc4f922d8ab622491dd44fc2c39afee9
  GIT_SHALLOW true
)

# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(googletest)

enable_testing()

file(GLOB TEST_SRC LIST_DIRECTORIES false src/*_test.cpp)

add_executable(chestillo_test src/board.cpp src/fen.cpp src/move.cpp ${TEST_SRC})
target_link_libraries(chestillo_test GTest::gtest_main)

include(GoogleTest)

gtest_discover_tests(chestillo_test)
