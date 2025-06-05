include(find_googletest)
include(find_tui_deps)
include(format)

file(GLOB_RECURSE TUI_SRC LIST_DIRECTORIES false **/*.cpp **/*.hpp)

format(tui "${TUI_SRC}")

add_library(tui
  STATIC
  src/color.cpp
  src/theme.cpp
  src/hooks.cpp
  src/mapping.cpp
  src/utility.cpp
  src/component/chessboard.cpp
  src/component/engine_settings.cpp
  src/component/input.cpp
  src/component/modal.cpp
  src/component/modal_view.cpp
  src/component/scroll_view.cpp
  src/component/square.cpp
  src/component/switch.cpp
  src/screen/analyze.cpp
  src/screen/home.cpp
)

if(TARGET tui_format)
  add_dependencies(tui tui_format)
endif()

target_include_directories(tui PUBLIC include)

target_compile_options(tui PRIVATE ${COMPILE_OPTIONS})
target_link_options(tui PRIVATE -O3)

target_link_libraries(tui
  PUBLIC uci
  PUBLIC ftxui::screen
  PUBLIC ftxui::dom
  PUBLIC ftxui::component
  PUBLIC nlohmann_json::nlohmann_json
)

add_executable(chesstillo_tui src/main.cpp)

target_compile_options(chesstillo_tui PRIVATE ${COMPILE_OPTIONS})
target_link_options(chesstillo_tui PRIVATE -O3)

target_link_libraries(chesstillo_tui
  PRIVATE tui
)

file(GLOB TUI_TEST_SRC LIST_DIRECTORIES false src/*_test.cpp src/**/*_test.cpp)

add_executable(tui_tests ${TUI_TEST_SRC})
target_include_directories(tui_tests PRIVATE include)

target_link_libraries(tui_tests
  PRIVATE tui
  PRIVATE GTest::gtest_main
  PRIVATE GTest::gmock
)

gtest_discover_tests(tui_tests)
