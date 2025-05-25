include(find_boost)
include(find_googletest)
include(format)

file(GLOB_RECURSE UCI_SRC LIST_DIRECTORIES false **/*.cpp **/*.hpp)

format(uci "${UCI_SRC}")

add_library(uci
  STATIC
  src/scanner.cpp
  src/parser.cpp
  src/uci.cpp
  src/engine.cpp
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

if(TARGET uci_format)
  add_dependencies(uci uci_format)
endif()

target_include_directories(uci PUBLIC include)

target_link_libraries(uci
  PUBLIC Boost::asio
  PUBLIC Boost::process
)

file(GLOB UCI_TEST_SRC LIST_DIRECTORIES false src/*_test.cpp)

add_executable(uci_tests ${UCI_TEST_SRC})
target_include_directories(uci_tests PRIVATE include)

target_link_libraries(uci_tests
  PRIVATE uci
  PRIVATE GTest::gtest_main
  PRIVATE GTest::gmock
)

gtest_discover_tests(uci_tests)
