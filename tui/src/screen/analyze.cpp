#include <functional>
#include <string>

#include <ftxui/component/component.hpp>
#include <ftxui/component/event.hpp>
#include <tui/components.hpp>
#include <tui/hooks.hpp>
#include <tui/screen/analyze.hpp>

using namespace tui;

screen::Analyze::Analyze() : fen_("hello world") { Add(InitContainer()); }

ftxui::Component &screen::Analyze::InitContainer() {
  auto fen_input = ftxui::Make<component::Input>(fen_);
  auto pgn_input = ftxui::Make<component::Input>(pgn_, true);

  auto config = ftxui::FlexboxConfig()
                    .Set(ftxui::FlexboxConfig::AlignItems::Center)
                    .Set(ftxui::FlexboxConfig::JustifyContent::SpaceBetween);

  auto component = ftxui::Container::Vertical({fen_input, pgn_input});

  return container_ =
             ftxui::Renderer(
                 component,
                 [=] {
                   return ftxui::vbox(
                       {ftxui::flexbox({ftxui::text("FEN"),
                                        ftxui::separatorEmpty(),
                                        fen_input->Render() | ftxui::flex_grow},
                                       config),
                        ftxui::flexbox({ftxui::text("PGN"),
                                        ftxui::separatorEmpty(),
                                        pgn_input->Render() | ftxui::flex_grow},
                                       config)});
                 }) |
             ftxui::color(ftxui::Color::GrayLight) |
             ftxui::size(ftxui::WIDTH, ftxui::Constraint::EQUAL, 100);
}

ftxui::Element screen::Analyze::OnRender() {
  static ftxui::FlexboxConfig config = {
      .direction = ftxui::FlexboxConfig::Direction::Column,
      .justify_content = ftxui::FlexboxConfig::JustifyContent::Center,
      .align_content = ftxui::FlexboxConfig::AlignContent::Center,
  };

  ftxui::Element content =
      ftxui::flexbox({ftxui::vbox({ftxui::text(fen_)}), container_->Render()},
                     config) |
      ftxui::color(ftxui::Color::GrayLight);

  return content;
}

bool screen::Analyze::OnEvent(ftxui::Event event) {
  return ftxui::ComponentBase::OnEvent(event);
}
