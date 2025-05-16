#include <ftxui/component/screen_interactive.hpp>
#include <tui/utility.hpp>

namespace tui {
ftxui::ScreenInteractive *ActiveScreen() {
  return ftxui::ScreenInteractive::Active();
}

void Navigate(const ftxui::Component &component) {
  auto screen = ftxui::ScreenInteractive::Fullscreen();

  screen.Loop(component);
}
} // namespace tui
