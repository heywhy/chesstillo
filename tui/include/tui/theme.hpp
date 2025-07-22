#ifndef TUI_THEME_HPP
#define TUI_THEME_HPP

#include <engine/engine.hpp>
#include <ftxui/screen/color.hpp>

namespace tui {
enum Square { LIGHT, DARK, FOCUSED, SELECTED, HOVER };

struct Theme {
  const ftxui::Color piece[2];
  const ftxui::Color square[5];

  static const Theme Default;
};
}  // namespace tui

#endif
