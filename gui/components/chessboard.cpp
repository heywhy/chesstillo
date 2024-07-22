#include <cstdlib>
#include <ftxui/component/component.hpp>
#include <ftxui/dom/node.hpp>

#include "chessboard.hpp"
#include "square.hpp"

Chessboard::Chessboard(std::shared_ptr<Board> board)
    : board_(board),
      selected_(static_cast<Square **>(std::malloc(sizeof(Square *)))) {
  for (int i = 0; i < 64; i++) {
    Add(ftxui::Make<Square>(this, selected_, i % 8, i / 8));
  }
}

ftxui::Element Chessboard::Render() {
  ftxui::Elements squares;

  for (int i = 7; i >= 0; i--) {
    ftxui::Elements row;

    for (int j = 0; j < 8; j++) {
      int index = 8 * i + j;

      row.push_back(children_[index]->Render());
    }

    squares.push_back(ftxui::hbox(row));
  }

  if (el_ != nullptr) {
    squares.push_back(el_);
  }

  return ftxui::vbox(squares) | ftxui::center | ftxui::reflect(box_);
}

bool Chessboard::OnEvent(ftxui::Event event) {
  if (event == ftxui::Event::a) {
    el_ = ftxui::text("hello world") | ftxui::bold |
          ftxui::color(ftxui::Color::Red);

    return true;
  }

  if (event == ftxui::Event::x) {
    el_ = nullptr;

    return true;
  }

  if (event.is_mouse() && event.mouse().button == ftxui::Mouse::Button::Left &&
      !box_.Contain(event.mouse().x, event.mouse().y)) {
    *selected_ = nullptr;

    return true;
  }

  return ftxui::ComponentBase::OnEvent(event);
}

void Chessboard::OnSelect(Square *square) {
  if (*selected_ == nullptr && square->IsEmpty()) {
    return;
  }

  if (*selected_ != nullptr && (*selected_)->IsEmpty()) {
    *selected_ = nullptr;

    return;
  }

  if (*selected_ == square) {
    *selected_ = nullptr;
  } else {
    *selected_ = square;
  }
}

void Chessboard::FillBoard() {
  for (ftxui::Component &component : children_) {
    Square *square = reinterpret_cast<Square *>(component.get());
    char piece = board_->PieceAtSquare(
        BitboardForSquare(square->GetFile(), square->GetRank()));

    square->SetPiece(piece);
  }
}
