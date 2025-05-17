include(find_googletest)
include(find_tui_deps)

add_executable(chesstillo_tui
  src/theme.cpp
  src/hooks.cpp
  src/utility.cpp
  src/component/chessboard.cpp
  src/component/input.cpp
  src/component/scroll_view.cpp
  src/component/square.cpp
  src/screen/analyze.cpp
  src/screen/home.cpp
  src/main.cpp
)

target_include_directories(chesstillo_tui PRIVATE include)

target_link_libraries(chesstillo_tui
  PRIVATE nlohmann_json::nlohmann_json
)

target_compile_options(chesstillo_tui PRIVATE ${COMPILE_OPTIONS})
target_link_options(chesstillo_tui PRIVATE -O3)

target_link_libraries(
  chesstillo_tui
  PRIVATE uci
  PRIVATE ftxui::screen
  PRIVATE ftxui::dom
  PRIVATE ftxui::component
)
