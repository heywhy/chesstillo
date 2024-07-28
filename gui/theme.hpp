#ifndef GUI_THEME_HPP

#define GUI_THEME_HPP

#include <ftxui/screen/color.hpp>

struct Theme {
  ftxui::Color const w_piece;
  ftxui::Color const b_piece;
  ftxui::Color const dark_square;
  ftxui::Color const light_square;
  ftxui::Color const selected_square;

  static const Theme Default;
};

#endif
