#ifndef TUI_THEME_HPP
#define TUI_THEME_HPP

#include <engine/engine.hpp>
#include <ftxui/screen/color.hpp>

namespace tui {
enum Square { LIGHT, DARK, FOCUSED, SELECTED };

struct Theme {
  const ftxui::Color piece[2];
  const ftxui::Color square[4];

  static const Theme Default;
};
}  // namespace tui

#endif
