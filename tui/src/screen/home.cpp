#include <ftxui/component/component.hpp>
#include <ftxui/component/event.hpp>
#include <tui/screen/analyze.hpp>
#include <tui/screen/home.hpp>
#include <tui/tui.hpp>

using namespace tui;

ftxui::Element screen::Home::OnRender() {
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

bool screen::Home::OnEvent(ftxui::Event event) {
  if (event == ftxui::Event::a) {
    ftxui::Component component = ftxui::Make<Analyze>();

    Navigate(component);

    return true;
  }

  return ftxui::ComponentBase::OnEvent(event);
}
