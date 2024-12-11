#ifndef TUI_CHESSBOARD_HPP
#define TUI_CHESSBOARD_HPP

#include <cstdint>

#include <chesstillo/position.hpp>
#include <chesstillo/utility.hpp>
#include <ftxui/component/component.hpp>
#include <ftxui/component/component_base.hpp>
#include <ftxui/component/mouse.hpp>
#include <ftxui/dom/node.hpp>

#include "components/square.hpp"
#include "contracts.hpp"

class Chessboard : public ftxui::ComponentBase, public SquareListener {
public:
  Chessboard(const Theme *theme, std::shared_ptr<Position> position,
             ChessboardListener *listener)
      : position_(position), selected_(-1), listener_(listener) {
    for (int i = 0; i < 64; i++) {
      Add(ftxui::Make<Square>(theme, this, i, &selected_));
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
    Square *square = static_cast<Square *>(children_[index].get());

    Square *selected = selected_ >= 0
                           ? static_cast<Square *>(children_[selected_].get())
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

    if (listener_ != nullptr) {
      Move move =
          DeduceMove(*position_, selected->GetIndex(), square->GetIndex());

      listener_->OnMove(move);

      FillBoard();
    }

    selected_ = -1;
  }

  void FillBoard() {
    for (ftxui::Component &component : children_) {
      char piece;
      Square *square = static_cast<Square *>(component.get());

      if (position_->PieceAt(&piece, square->GetIndex())) {
        square->SetPiece(piece);
      } else {
        square->SetPiece('\0');
      }
    }
  }

private:
  ftxui::Box box_;
  std::shared_ptr<Position> position_;
  int8_t selected_;
  ChessboardListener *listener_;
};
#endif
