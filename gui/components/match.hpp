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
#include "components/scroll_view.hpp"

class Match : public ftxui::ComponentBase {
public:
  Match() : Match(std::make_shared<Board>()) {}

  Match(std::shared_ptr<Board> board)
      : board_(board), chessboard_(ftxui::Make<Chessboard>(board_)),
        moves_board_(ftxui::Make<MovesBoard>(board_)),
        scroll_view_(ftxui::Make<ScrollView>(moves_board_)) {
    Add(chessboard_);
    Add(scroll_view_);

    ApplyFen(*board_, START_FEN);
    GetChessboard()->FillBoard();
  }

  ftxui::Element Render() override {
    ftxui::FlexboxConfig config;
    ftxui::Dimensions dimensions = ftxui::Dimension::Full();

    config.Set(ftxui::FlexboxConfig::Direction::Column);
    config.Set(ftxui::FlexboxConfig::AlignContent::Center);
    config.Set(ftxui::FlexboxConfig::JustifyContent::Center);

    std::string turn(board_->GetTurn() == WHITE ? "white" : "black");
    ftxui::Elements menus = {ftxui::text("[a] Analyze"), ftxui::separator(),
                             ftxui::text("[r] Restart"), ftxui::separator(),
                             ftxui::text("[q] Quit")};

    ftxui::Element box =
        ftxui::hbox(
            {chessboard_->Render(), ftxui::separatorEmpty(),
             ftxui::separatorEmpty(),
             ftxui::vbox(
                 {ftxui::hbox({ftxui::text("Moves") | ftxui::bold,
                               ftxui::filler(), ftxui::text(turn + "'s turn")}),
                  ftxui::separator(),

                  scroll_view_->Render()}) |
                 ftxui::center

            }) |
        ftxui::center;

    box |= ftxui::size(ftxui::WIDTH, ftxui::EQUAL, dimensions.dimx / 2) |
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
  std::shared_ptr<Board> board_;
  ftxui::Component chessboard_;
  ftxui::Component moves_board_;
  ftxui::Component scroll_view_;

  Chessboard *GetChessboard() {
    return dynamic_cast<Chessboard *>(chessboard_.get());
  }
};

#endif
