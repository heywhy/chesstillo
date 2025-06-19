include(find_engine_deps)
include(find_googletest)
include(format)

file(GLOB_RECURSE ENGINE_SRC LIST_DIRECTORIES false **/*.cpp **/*.hpp)

format(engine "${ENGINE_SRC}")

add_library(engine
  STATIC
  src/initializer.cpp
  src/utility.cpp
  src/board.cpp
  src/position.cpp
  src/fen.cpp
  src/fill.cpp
  # src/game.cpp
  src/move.cpp
  src/move_gen.cpp
  src/evaluation.cpp
  src/options.cpp
  src/search.cpp
  src/scheduler.cpp
  src/threads.cpp
  # src/transposition.cpp
  src/ybwc.cpp
)

if(TARGET engine_format)
  add_dependencies(engine engine_format)
endif()

target_include_directories(engine PUBLIC include)

target_compile_options(engine PRIVATE ${COMPILE_OPTIONS})
target_link_options(engine PRIVATE -O3)

target_link_libraries(engine
  PUBLIC magic_bits
  PRIVATE uci
)

add_executable(chesstillo src/main.cpp)

target_compile_options(chesstillo PRIVATE ${COMPILE_OPTIONS})
target_link_options(chesstillo PRIVATE -O3)

target_link_libraries(chesstillo PRIVATE engine)

file(
  GLOB
  ENGINE_TEST_SRC
  LIST_DIRECTORIES false
  src/*_test.cpp
  src/**/*_test.cpp
)

add_executable(engine_tests ${ENGINE_TEST_SRC})
target_include_directories(engine_tests PRIVATE include)

target_link_libraries(engine_tests
  PRIVATE engine
  PRIVATE GTest::gtest_main
  PRIVATE GTest::gmock
)

gtest_discover_tests(engine_tests)
