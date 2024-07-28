#ifndef GUI_CHESSBOARD_HPP
#define GUI_CHESSBOARD_HPP

#include <chesstillo/board.hpp>
#include <cstdint>
#include <ftxui/component/component.hpp>
#include <ftxui/component/component_base.hpp>
#include <ftxui/component/mouse.hpp>
#include <ftxui/dom/node.hpp>

#include "components/square.hpp"
#include "utils.hpp"

class Chessboard : public ftxui::ComponentBase, public OnSelectSquare {
public:
  Chessboard(std::shared_ptr<Board> board) : board_(board), selected_(-1) {
    for (int i = 0; i < 64; i++) {
      Add(ftxui::Make<Square>(this, i, &selected_));
    }
  }

  ftxui::Element Render() override {
    ftxui::Elements squares;

    for (int i = 7; i >= 0; i--) {
      ftxui::Elements row;

      for (int j = 0; j < 8; j++) {
        int index = 8 * i + j;

        row.push_back(children_[index]->Render());
      }

      squares.push_back(ftxui::hbox(row));
    }

    return ftxui::vbox(squares) | ftxui::center | ftxui::reflect(box_);
  }

  bool OnEvent(ftxui::Event event) override {
    if (event.is_mouse() &&
        event.mouse().button == ftxui::Mouse::Button::Left &&
        !box_.Contain(event.mouse().x, event.mouse().y)) {
      selected_ = -1;

      return true;
    }

    return ftxui::ComponentBase::OnEvent(event);
  }

  void OnSelect(std::uint8_t index) override {
    Square *square = dynamic_cast<Square *>(children_[index].get());

    Square *selected = selected_ >= 0
                           ? dynamic_cast<Square *>(children_[selected_].get())
                           : nullptr;

    if (selected == nullptr && square->IsEmpty()) {
      return;
    }

    if (selected != nullptr && selected->IsEmpty()) {
      selected_ = -1;
      return;
    }

    if (selected == square) {
      selected_ = -1;
      return;
    }

    if (selected == nullptr) {
      selected_ = index;
      return;
    }

    Piece piece;

    if (board_->PieceAtSquare(selected->bitboard, &piece)) {
      Move move(selected->bitboard, square->bitboard, board_->GetTurn(), piece);

      board_->ApplyMove(move);

      FillBoard();
    }

    selected_ = -1;
  }

  void FillBoard() {
    for (ftxui::Component &component : children_) {
      char piece;
      Square *square = dynamic_cast<Square *>(component.get());

      if (board_->PieceAtSquare(square->bitboard, &piece)) {
        square->SetPiece(piece);
      } else {
        square->SetPiece('\0');
      }
    }
  }

private:
  ftxui::Box box_;
  std::shared_ptr<Board> board_;
  std::int8_t selected_;
};

#endif
