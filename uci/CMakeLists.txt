include(find_googletest)

add_library(uci
  STATIC
  scanner.cpp
  parser.cpp
  uci.cpp
  exprs/go.cpp
  exprs/position.cpp
  exprs/setoption.cpp
  exprs/register.cpp
  exprs/id.cpp
  exprs/bestmove.cpp
  exprs/copyprotection.cpp
  exprs/registration.cpp
  exprs/info.cpp
  exprs/option.cpp
)

target_include_directories(uci PUBLIC include)

file(GLOB UCI_TEST_SRC LIST_DIRECTORIES false *_test.cpp)

add_executable(tests ${UCI_TEST_SRC})
target_include_directories(tests PRIVATE include)

target_link_libraries(tests
  PRIVATE uci
  PRIVATE GTest::gtest_main
  PRIVATE GTest::gmock
)

gtest_discover_tests(tests)
