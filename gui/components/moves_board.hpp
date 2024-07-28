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

    elements.reserve(moves.size() / 2);

    int index = 1;
    auto it = moves.rbegin();

    while (it != moves.rend()) {
      ftxui::Elements texts;
      std::string move_number = std::to_string(index) + ". ";

      texts.push_back(ftxui::text(move_number) | ftxui::center |
                      ftxui::size(ftxui::WIDTH, ftxui::EQUAL, 5));

      char text[5];

      if (MoveToString(*it, text)) {
        texts.push_back(ftxui::text(text) |
                        ftxui::size(ftxui::WIDTH, ftxui::EQUAL, 10));

        it++;
      }

      if (it != moves.rend() && MoveToString(*it, text)) {
        texts.push_back(ftxui::text(text) |
                        ftxui::size(ftxui::WIDTH, ftxui::EQUAL, 10));

        it++;
      }

      if (texts.size() > 0) {
        elements.push_back(ftxui::hbox(texts));
      }

      ++index;
    }

    return ftxui::vbox(elements) |
           ftxui::size(ftxui::HEIGHT, ftxui::EQUAL, 22) |
           ftxui::size(ftxui::WIDTH, ftxui::GREATER_THAN, 30);
  }

  bool Focusable() const override { return true; }

private:
  std::shared_ptr<Board> board_;
};

#endif
