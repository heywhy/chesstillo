#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>

#include "screens/home.hpp"
#include "utils.hpp"

int main() {
  ftxui::Component component = ftxui::Make<Home>();
  ftxui::ScreenInteractive screen = ftxui::ScreenInteractive::Fullscreen();

  HookQuitEvent(component);

  screen.Loop(component);

  return 0;
}
