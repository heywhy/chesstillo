#include <ftxui/component/component.hpp>
#include <ftxui/component/event.hpp>
#include <ftxui/component/screen_interactive.hpp>

#include "tui/hooks.hpp"

namespace tui {
namespace hooks {

ftxui::ComponentDecorator QuitEvent() {
  return [](ftxui::Component child) {
    child |= ftxui::CatchEvent([&](ftxui::Event event) {
      auto screen = ftxui::ScreenInteractive::Active();

      if (event == ftxui::Event::q && screen != nullptr) {
        screen->ExitLoopClosure()();

        return true;
      }

      return false;
    });

    return child;
  };
}

}  // namespace hooks
}  // namespace tui
