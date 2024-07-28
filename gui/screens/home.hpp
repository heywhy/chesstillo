#ifndef GUI_HOME_HPP

#define GUI_HOME_HPP

#include <ftxui/component/component.hpp>
#include <ftxui/component/component_base.hpp>
#include <ftxui/component/event.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/node.hpp>

#include "components/match.hpp"
#include "theme.hpp"
#include "utils.hpp"

class Home : public ftxui::ComponentBase {
public:
  ftxui::Element Render() override {
    ftxui::FlexboxConfig config;

    config.Set(ftxui::FlexboxConfig::Direction::Column);
    config.Set(ftxui::FlexboxConfig::AlignContent::Center);
    config.Set(ftxui::FlexboxConfig::JustifyContent::Center);

    ftxui::Element content =
        ftxui::flexbox({ftxui::text("Welcome to Chestillo") | ftxui::bold,
                        ftxui::separatorEmpty(), ftxui::text("[n] New game"),
                        ftxui::text("[q] Quit")},
                       config) |
        ftxui::border;

    return content;
  }

  bool OnEvent(ftxui::Event event) override {
    if (event == ftxui::Event::n) {
      OnNewGame();

      return false;
    }

    return ftxui::ComponentBase::OnEvent(event);
  }

private:
  void OnNewGame() {
    ftxui::Component component = ftxui::Make<Match>(&Theme::Default);
    ftxui::ScreenInteractive screen = ftxui::ScreenInteractive::Fullscreen();

    HookQuitEvent(component);

    screen.Loop(component);
  }
};

#endif
