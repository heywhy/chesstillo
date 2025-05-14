include(find_googletest)

add_library(uci
  STATIC
  src/scanner.cpp
  src/parser.cpp
  src/uci.cpp
  src/command/debug.cpp
  src/command/go.cpp
  src/command/position.cpp
  src/command/setoption.cpp
  src/command/register.cpp
  src/command/id.cpp
  src/command/bestmove.cpp
  src/command/copyprotection.cpp
  src/command/registration.cpp
  src/command/info.cpp
  src/command/option.cpp
)

target_include_directories(uci PUBLIC include)

file(GLOB UCI_TEST_SRC LIST_DIRECTORIES false src/*_test.cpp)

add_executable(tests ${UCI_TEST_SRC})
target_include_directories(tests PRIVATE include)

target_link_libraries(tests
  PRIVATE uci
  PRIVATE GTest::gtest_main
  PRIVATE GTest::gmock
)

gtest_discover_tests(tests)
