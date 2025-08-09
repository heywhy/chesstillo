#include <ftxui/component/component_base.hpp>
#include <ftxui/component/event.hpp>

#include "tui/screen/analyze.hpp"
#include "tui/screen/home.hpp"
#include "tui/utils.hpp"

namespace tui {
namespace screen {

Home::Home(const Theme &theme) : theme_(theme) {}

ftxui::Element Home::OnRender() {
  static ftxui::FlexboxConfig config = {
      .direction = ftxui::FlexboxConfig::Direction::Column,
      .justify_content = ftxui::FlexboxConfig::JustifyContent::Center,
      .align_content = ftxui::FlexboxConfig::AlignContent::Center,
  };

  ftxui::Element content =
      ftxui::flexbox({ftxui::text("Welcome to Chesstillo") | ftxui::bold,
                      ftxui::separatorEmpty(), ftxui::text("[n] New game"),
                      ftxui::separatorEmpty(), ftxui::text("[a] Analyse game"),
                      ftxui::separatorEmpty(), ftxui::text("[i] Import game"),
                      ftxui::separatorEmpty(), ftxui::text("[q] Quit")},
                     config) |
      ftxui::border;

  return content;
}

bool Home::OnEvent(ftxui::Event event) {
  if (event == ftxui::Event::a) {
    ftxui::Component component = tui::Make<Analyze>(theme_);

    Navigate(component);

    return true;
  }

  return ftxui::ComponentBase::OnEvent(event);
}

}  // namespace screen
}  // namespace tui
