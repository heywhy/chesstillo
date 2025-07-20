#ifndef TUI_THEME_HPP
#define TUI_THEME_HPP

#include <engine/engine.hpp>
#include <ftxui/screen/color.hpp>

namespace tui {
enum Square { LIGHT, DARK };

struct Theme {
  const ftxui::Color piece[2];
  const ftxui::Color square[2];
  const ftxui::Color focused_square;
  const ftxui::Color selected_square;

  static const Theme Default;
};
}  // namespace tui

#endif
