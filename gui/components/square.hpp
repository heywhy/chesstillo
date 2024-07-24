#ifndef GUI_SQUARE_HPP
#define GUI_SQUARE_HPP

#include <chesstillo/board.hpp>
#include <chesstillo/constants.hpp>
#include <chesstillo/types.hpp>
#include <ftxui/component/component_base.hpp>
#include <ftxui/component/event.hpp>

#include "utils.hpp"

class Square : public ftxui::ComponentBase {
public:
  Square(OnSelectSquare *observer, std::uint8_t index, std::int8_t *selected)
      : observer_(observer), index_(index), selected_(selected), piece_('\0') {}

  void SetPiece(char piece) { piece_ = piece; }
  bool IsEmpty() { return piece_ == '\0'; }

  Bitboard ToBitboard() { return BITBOARD_FOR_SQUARE(index_); }

  ftxui::Element Render() override {
    ftxui::Elements elements;

    if (piece_ != '\0') {
      std::string p(1, piece_);

      elements.push_back({ftxui::text(p)});
    }

    ftxui::Element square = ftxui::dbox(elements | ftxui::center);

    square |= ftxui::size(ftxui::WIDTH, ftxui::EQUAL, 4) |
              ftxui::size(ftxui::HEIGHT, ftxui::EQUAL, 2) | ftxui::center;

    if (*selected_ != index_ && ToBitboard() & kDarkSquares) {
      square |= ftxui::bgcolor(ftxui::Color::Black) |
                ftxui::color(ftxui::Color::White);
    } else if (*selected_ != index_) {
      square |= ftxui::bgcolor(ftxui::Color::White) |
                ftxui::color(ftxui::Color::Black);
    } else {
      square |= ftxui::bgcolor(ftxui::Color::Green4);
    }

    return square | ftxui::reflect(box_);
  }

  bool OnEvent(ftxui::Event event) override {
    if (event.is_mouse()) {
      ftxui::Mouse mouse = event.mouse();

      if (mouse.motion == ftxui::Mouse::Motion::Pressed &&
          mouse.button == ftxui::Mouse::Button::Left &&
          box_.Contain(mouse.x, mouse.y)) {
        observer_->OnSelect(index_);

        return true;
      }
    }

    return ftxui::ComponentBase::OnEvent(event);
  }

private:
  ftxui::Box box_;
  OnSelectSquare *observer_;
  std::uint8_t index_;
  std::int8_t *selected_;
  char piece_;
};

#endif
