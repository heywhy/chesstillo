#ifndef TUI_THEME_HPP
#define TUI_THEME_HPP

#include <ftxui/screen/color.hpp>

namespace tui {
struct Theme {
  const ftxui::Color w_piece;
  const ftxui::Color b_piece;
  const ftxui::Color dark_square;
  const ftxui::Color light_square;
  const ftxui::Color selected_square;

  static const Theme Default;
};
} // namespace tui

#endif
