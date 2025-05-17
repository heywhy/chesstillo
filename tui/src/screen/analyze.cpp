#include <functional>
#include <string>

#include <ftxui/component/component.hpp>
#include <tui/components.hpp>
#include <tui/hooks.hpp>
#include <tui/screen/analyze.hpp>
#include <tui/theme.hpp>

namespace tui {
namespace screen {

Analyze::Analyze(const Theme &theme) : theme_(theme) { Add(MakeContainer()); }

ftxui::Component Analyze::MakeContainer() {
  auto fen_input = ftxui::Make<component::Input>(fen_);
  auto pgn_input = ftxui::Make<component::Input>(pgn_, true);
  auto chessboard = ftxui::Make<component::Chessboard>(theme_);

  auto config = ftxui::FlexboxConfig()
                    .Set(ftxui::FlexboxConfig::AlignItems::Center)
                    .Set(ftxui::FlexboxConfig::JustifyContent::SpaceBetween);

  auto component =
      ftxui::Container::Vertical({chessboard, fen_input, pgn_input});

  return ftxui::Renderer(
             component,
             [=] {
               return ftxui::vbox(
                   {chessboard->Render(),
                    ftxui::flexbox({ftxui::text("FEN"), ftxui::separatorEmpty(),
                                    fen_input->Render() | ftxui::flex_grow},
                                   config),
                    ftxui::flexbox({ftxui::text("PGN"), ftxui::separatorEmpty(),
                                    pgn_input->Render() | ftxui::flex_grow},
                                   config)});
             }) |
         ftxui::color(ftxui::Color::GrayLight) |
         ftxui::size(ftxui::WIDTH, ftxui::Constraint::EQUAL, 100);
}

ftxui::Element Analyze::OnRender() {
  static ftxui::FlexboxConfig config = {
      .direction = ftxui::FlexboxConfig::Direction::Column,
      .justify_content = ftxui::FlexboxConfig::JustifyContent::Center,
      .align_content = ftxui::FlexboxConfig::AlignContent::Center,
  };

  ftxui::Element content = ftxui::flexbox({ChildAt(0)->Render()}, config) |
                           ftxui::color(ftxui::Color::GrayLight);

  return content;
}

bool Analyze::OnEvent(ftxui::Event event) {
  return ftxui::ComponentBase::OnEvent(event);
}

} // namespace screen
} // namespace tui
