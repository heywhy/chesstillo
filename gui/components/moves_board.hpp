#ifndef GUI_MOVES_HPP

#define GUI_MOVES_HPP

#include <chesstillo/board.hpp>
#include <chesstillo/types.hpp>
#include <ftxui/component/component_base.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/dom/node.hpp>
#include <list>
#include <memory>
#include <string>

class MovesBoard : public ftxui::ComponentBase {
public:
  MovesBoard(std::shared_ptr<Board> board) : board_(board) {}

  ftxui::Element Render() override {
    ftxui::Elements elements;

    std::list<Move> moves = board_->GetMoves();
    std::string turn(board_->GetTurn() == WHITE ? "white" : "black");

    elements.reserve(moves.size() / 2);

    int index = 1;
    auto it = moves.rbegin();

    while (it != moves.rend()) {
      ftxui::Elements texts;
      std::string move_number = std::to_string(index) + ". ";

      texts.push_back(ftxui::text(move_number));

      char text[5];

      if (MoveToString(*it, text)) {
        texts.push_back(ftxui::text(text) | ftxui::flex_grow);

        it++;
      }

      if (it != moves.rend() && MoveToString(*it, text)) {
        texts.push_back(ftxui::text(text) | ftxui::flex_grow);

        it++;
      }

      if (texts.size() > 0) {
        elements.push_back(ftxui::hbox(texts));
      }

      ++index;
    }

    return ftxui::vbox(
               {ftxui::hbox({ftxui::text("Moves") | ftxui::bold,
                             ftxui::filler(), ftxui::text(turn + "'s turn")}),
                ftxui::separator(),
                ftxui::vbox(elements) | ftxui::vscroll_indicator |
                    ftxui::frame}) |
           ftxui::size(ftxui::HEIGHT, ftxui::EQUAL, 17) |
           ftxui::size(ftxui::WIDTH, ftxui::GREATER_THAN, 30);
  }

  bool Focusable() const override { return true; }

private:
  std::shared_ptr<Board> board_;
};

#endif
