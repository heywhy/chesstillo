#include <cstddef>
#include <utility>

#include <ftxui/component/component_base.hpp>
#include <ftxui/component/event.hpp>

#include "engine/square.hpp"

#include "tui/component/chessboard.hpp"
#include "tui/component/square.hpp"
#include "tui/constants.hpp"
#include "tui/theme.hpp"
#include "tui/utility.hpp"

namespace tui {
namespace component {

Chessboard::Chessboard(const Theme &theme, OnSelect on_select)
    : selector_(12), on_select_(std::move(on_select)) {
  for (int i = 0; i < 64; i++) {
    Add(tui::Make<component::Square>(theme, i, on_select_));
  }
}

void Chessboard::ToggleSquare(Square *square) {
  for (std::size_t i = 0; i < children_.size(); i++) {
    if (children_[i].get() == square) {
      square->Toggle();
      break;
    }
  }
}

ftxui::Element Chessboard::OnRender() {
  ftxui::Elements squares;

  for (int i = 7; i >= 0; i--) {
    ftxui::Elements row;

    for (int j = 0; j < 8; j++) {
      int index = engine::square::From(j, i);

      row.push_back(ChildAt(index)->Render());
    }

    squares.push_back(ftxui::hbox(row));
  }

  return ftxui::vbox(squares) | ftxui::reflect(box_);
}

bool Chessboard::OnEvent(ftxui::Event event) {
  if (event.is_mouse()) {
    return OnMouseEvent(event);
  }

  if (!Focused()) {
    return false;
  }

  return OnKeyEvent(event);
}

bool Chessboard::OnMouseEvent(ftxui::Event &event) {
  const ftxui::Mouse &mouse = event.mouse();

  if (!box_.Contain(mouse.x, mouse.y) ||
      mouse.button != ftxui::Mouse::Button::Left) {
    return false;
  }

  if (!Focused()) {
    TakeFocus();
  }

  return ftxui::ComponentBase::OnEvent(event);
}

bool Chessboard::OnKeyEvent(ftxui::Event &event) {
  int old_selected = selector_;

  if (event.is_character() && event.input() == kSpaceChar) {
    auto square = dynamic_cast<Square *>(ActiveChild().get());

    on_select_(square);

    return true;
  }

  if (event == ftxui::Event::ArrowUp || event == ftxui::Event::k) {
    MoveSelector(8);
  }

  if (event == ftxui::Event::ArrowDown || event == ftxui::Event::j) {
    MoveSelector(-8);
  }

  if (event == ftxui::Event::ArrowLeft || event == ftxui::Event::h) {
    MoveSelector(-1);
  }

  if (event == ftxui::Event::ArrowRight || event == ftxui::Event::l) {
    MoveSelector(1);
  }

  selector_ = std::max(0, std::min(int(children_.size()) - 1, selector_));

  return old_selected != selector_;
}

ftxui::Component Chessboard::ActiveChild() {
  return selector_ >= 0 ? children_[selector_] : nullptr;
}

void Chessboard::SetActiveChild(ftxui::ComponentBase *child) {
  for (std::size_t i = 0; i < children_.size(); ++i) {
    if (children_[i].get() == child) {
      selector_ = static_cast<int>(i);
      break;
    }
  }
}

void Chessboard::MoveSelector(int dir) {
  for (int i = selector_ + dir; i >= 0 && i < int(children_.size()); i += dir) {
    if (children_[i]->Focusable()) {
      selector_ = i;
      return;
    }
  }
}

}  // namespace component
}  // namespace tui
