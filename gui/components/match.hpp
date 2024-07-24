#ifndef GUI_MATCH_HPP

#define GUI_MATCH_HPP

#include <chesstillo/board.hpp>
#include <chesstillo/fen.hpp>
#include <ftxui/component/component.hpp>
#include <ftxui/component/component_base.hpp>
#include <ftxui/component/event.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/dom/node.hpp>
#include <memory>

#include "components/chessboard.hpp"
#include "components/moves_board.hpp"

class Match : public ftxui::ComponentBase {
public:
  Match() : Match(std::make_shared<Board>()) {}

  Match(std::shared_ptr<Board> board)
      : board_(board), chessboard_(ftxui::Make<Chessboard>(board)),
        moves_board_(ftxui::Make<MovesBoard>(board)) {
    Add(chessboard_);
    Add(moves_board_);

    ApplyFen(*board_, START_FEN);
    GetChessboard()->FillBoard();
  }

  ftxui::Element Render() override {
    ftxui::FlexboxConfig config;
    ftxui::Dimensions dimensions = ftxui::Dimension::Full();

    config.Set(ftxui::FlexboxConfig::Direction::Column);
    config.Set(ftxui::FlexboxConfig::AlignContent::Center);
    config.Set(ftxui::FlexboxConfig::JustifyContent::Center);

    ftxui::Elements menus = {ftxui::text("[a] Analyze"), ftxui::separator(),
                             ftxui::text("[r] Restart"), ftxui::separator(),
                             ftxui::text("[q] Quit")};

    ftxui::Element box =
        ftxui::hbox({chessboard_->Render(), ftxui::separatorEmpty(),
                     ftxui::separatorEmpty(),
                     moves_board_->Render() | ftxui::center}) |
        ftxui::center |
        ftxui::size(ftxui::WIDTH, ftxui::EQUAL, dimensions.dimx / 2) |
        ftxui::size(ftxui::HEIGHT, ftxui::EQUAL, dimensions.dimy / 2);

    ftxui::Element content = ftxui::flexbox({box}, config);

    return ftxui::window(ftxui::hbox(menus), content);
  }

  bool OnEvent(ftxui::Event event) override {
    if (event == ftxui::Event::r) {
      ApplyFen(*board_, START_FEN);

      GetChessboard()->FillBoard();

      return true;
    }

    return ftxui::ComponentBase::OnEvent(event);
  }

private:
  ftxui::Component chessboard_;
  ftxui::Component moves_board_;
  std::shared_ptr<Board> board_;

  Chessboard *GetChessboard() {
    return dynamic_cast<Chessboard *>(chessboard_.get());
  }
};

#endif
