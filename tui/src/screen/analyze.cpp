#include <string>

#include <ftxui/component/component.hpp>
#include <tui/components.hpp>
#include <tui/hooks.hpp>
#include <tui/screen/analyze.hpp>
#include <tui/theme.hpp>

#define START_FEN "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"

namespace tui {
namespace screen {

Analyze::Analyze(const Theme &theme)
    : theme_(theme), fen_(START_FEN),
      engine_(uci::FindExecutable("stockfish"), this) {
  Add(MakeContainer());
}

ftxui::Component Analyze::MakeContainer() {
  auto fen_input = ftxui::Make<component::Input>("FEN", fen_);
  auto pgn_input = ftxui::Make<component::Input>("PGN", pgn_, true);
  auto chessboard = ftxui::Make<component::Chessboard>(theme_);

  return ftxui::Container::Vertical({chessboard, fen_input, pgn_input});
}

ftxui::Element Analyze::OnRender() {
  static ftxui::FlexboxConfig config = {
      .direction = ftxui::FlexboxConfig::Direction::Column,
      .justify_content = ftxui::FlexboxConfig::JustifyContent::Center,
      .align_content = ftxui::FlexboxConfig::AlignContent::Center,
  };

  ftxui::Component container = ChildAt(0);

  ftxui::Element content =
      ftxui::flexbox(
          {ftxui::gridbox(
              {{container->ChildAt(0)->Render(),
                ftxui::gaugeUp(0.5) | ftxui::color(ftxui::Color::RosyBrown) |
                    ftxui::bgcolor(ftxui::Color::GrayLight),
                ftxui::vbox({ftxui::text("moves block")})},
               {ftxui::separatorEmpty()},
               {container->ChildAt(1)->Render() |
                ftxui::size(ftxui::WIDTH, ftxui::LESS_THAN, 62)},
               {container->ChildAt(2)->Render() |
                ftxui::size(ftxui::WIDTH, ftxui::LESS_THAN, 62)}})},
          config) |
      ftxui::color(ftxui::Color::GrayLight) | ftxui::vscroll_indicator |
      ftxui::frame;

  return content;
}

bool Analyze::OnEvent(ftxui::Event event) {
  return ftxui::ComponentBase::OnEvent(event);
}

void Analyze::Handle(command::Input *) {}

void Analyze::Handle(command::ID *) {}

void Analyze::Handle(command::BestMove *) {}

void Analyze::Handle(command::CopyProtection *) {}

void Analyze::Handle(command::Registration *) {}

void Analyze::Handle(command::Info *) {}

void Analyze::Handle(command::Option *) {}
} // namespace screen
} // namespace tui
