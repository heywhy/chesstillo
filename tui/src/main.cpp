#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <uci/uci.hpp>

#include <tui/tui.hpp>

using namespace tui;

int main() {
  Theme theme = Theme::Default;
  ftxui::Component home = ftxui::Make<screen::Home>(theme);

  home |= hooks::QuitEvent();

  Navigate(home);

  return 0;
}
