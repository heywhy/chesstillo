#ifndef TUI_UTILITY_HPP
#define TUI_UTILITY_HPP

#include <ftxui/component/component_base.hpp>
#include <ftxui/component/screen_interactive.hpp>

#define NAVIGATE(component)                                                    \
  {                                                                            \
    auto screen = ftxui::ScreenInteractive::Fullscreen();                      \
    screen.Loop(component);                                                    \
  }

namespace tui {
ftxui::ScreenInteractive *ActiveScreen();

void Navigate(const ftxui::Component &component);
} // namespace tui

#endif
