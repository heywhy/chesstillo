#include <ftxui/component/component.hpp>
#include <ftxui/component/event.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <tui/hooks.hpp>
#include <tui/utility.hpp>

using namespace tui;

ftxui::ComponentDecorator hooks::QuitEvent() {
  return [](ftxui::Component child) {
    child |= ftxui::CatchEvent([&](ftxui::Event event) {
      auto screen = ActiveScreen();

      if (event == ftxui::Event::q && screen != nullptr) {
        screen->ExitLoopClosure()();

        return true;
      }

      return false;
    });

    return child;
  };
}
