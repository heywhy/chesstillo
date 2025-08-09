#include <ftxui/component/component_base.hpp>

#include "tui/hooks.hpp"
#include "tui/screen/home.hpp"
#include "tui/theme.hpp"
#include "tui/utility.hpp"

using namespace tui;

int main() {
  Theme theme = Theme::Default;
  ftxui::Component home = tui::Make<screen::Home>(theme);

  home |= hooks::QuitEvent();

  Navigate(home);

  return 0;
}
