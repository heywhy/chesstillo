#include <ftxui/component/component_base.hpp>
#include <uci/uci.hpp>

#include <tui/tui.hpp>

using namespace tui;

int main() {
  Theme theme = Theme::Default;
  ftxui::Component home = tui::Make<screen::Home>(theme);

  home |= hooks::QuitEvent();

  Navigate(home);

  return 0;
}
