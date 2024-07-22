#include <ftxui/component/component.hpp>
#include <ftxui/component/event.hpp>
#include <ftxui/component/screen_interactive.hpp>

#include "utils.hpp"

void HookQuitEvent(ftxui::Component &component) {
  component |= ftxui::CatchEvent([&](ftxui::Event event) {
    auto screen = ftxui::ScreenInteractive::Active();

    if (event == ftxui::Event::q && screen != nullptr) {
      screen->ExitLoopClosure()();

      return true;
    }

    return false;
  });
}
