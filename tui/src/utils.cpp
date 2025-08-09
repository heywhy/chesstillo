#include <ftxui/component/component_base.hpp>
#include <ftxui/component/screen_interactive.hpp>

#include "tui/utils.hpp"

namespace tui {
void Navigate(const ftxui::Component &component) {
  auto screen = ftxui::ScreenInteractive::Fullscreen();

  screen.Loop(component);
}
}  // namespace tui
