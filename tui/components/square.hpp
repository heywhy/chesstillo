#ifndef TUI_SQUARE_HPP
#define TUI_SQUARE_HPP

#include <tuple>

#include <chesstillo/board.hpp>
#include <chesstillo/constants.hpp>
#include <chesstillo/types.hpp>
#include <ftxui/component/component_base.hpp>
#include <ftxui/component/event.hpp>
#include <ftxui/screen/color.hpp>

#include "contracts.hpp"
#include "fonts.hpp"
#include "theme.hpp"

class Square : public ftxui::ComponentBase {
public:
  Square(const Theme *theme, SquareListener *listener, std::uint8_t index,
         std::int8_t *selected)
      : theme_(theme), listener_(listener), index_(index),
        bitboard_(BITBOARD_FOR_SQUARE(index)), selected_(selected),
        piece_('\0') {}

  std::uint8_t GetIndex() { return index_; }
  void SetPiece(char piece) { piece_ = piece; }
  bool IsEmpty() { return piece_ == '\0'; }

  ftxui::Element Render() override {
    ftxui::Elements elements;
    std::tuple<const char *, ftxui::Color> piece;

    if (ContainedPiece(&piece)) {
      ftxui::Element content =
          ftxui::text(std::get<0>(piece)) | ftxui::color(std::get<1>(piece));

      elements.push_back(content);
    }

    ftxui::Element square = ftxui::vbox(elements) | ftxui::center;

    square |= ftxui::size(ftxui::WIDTH, ftxui::EQUAL, 7) |
              ftxui::size(ftxui::HEIGHT, ftxui::EQUAL, 3);

    if (*selected_ != index_) {
      ftxui::Color color =
          bitboard_ & kDarkSquares ? theme_->dark_square : theme_->light_square;

      square |= ftxui::bgcolor(color);
    } else {
      square |= ftxui::bgcolor(theme_->selected_square);
    }

    return square | ftxui::reflect(box_);
  }

  bool OnEvent(ftxui::Event event) override {
    if (event.is_mouse() && listener_ != nullptr) {
      ftxui::Mouse mouse = event.mouse();

      if (mouse.motion == ftxui::Mouse::Motion::Pressed &&
          mouse.button == ftxui::Mouse::Button::Left &&
          box_.Contain(mouse.x, mouse.y)) {
        listener_->OnSelect(index_);

        return true;
      }
    }

    return ftxui::ComponentBase::OnEvent(event);
  }

private:
  ftxui::Box box_;
  const Theme *theme_;
  SquareListener *listener_;
  std::uint8_t index_;
  Bitboard bitboard_;
  std::int8_t *selected_;
  char piece_;

  bool ContainedPiece(std::tuple<const char *, ftxui::Color> *result) {
    switch (piece_) {
    case 'r':
      *result = {kRook, theme_->b_piece};
      break;

    case 'R':
      *result = {kRook, theme_->w_piece};
      break;

    case 'n':
      *result = {kKnight, theme_->b_piece};
      break;

    case 'N':
      *result = {kKnight, theme_->w_piece};
      break;

    case 'b':
      *result = {kBishop, theme_->b_piece};
      break;

    case 'B':
      *result = {kBishop, theme_->w_piece};
      break;

    case 'k':
      *result = {kKing, theme_->b_piece};
      break;

    case 'K':
      *result = {kKing, theme_->w_piece};
      break;

    case 'q':
      *result = {kQueen, theme_->b_piece};
      break;

    case 'Q':
      *result = {kQueen, theme_->w_piece};
      break;

    case 'p':
      *result = {kPawn, theme_->b_piece};
      break;

    case 'P':
      *result = {kPawn, theme_->w_piece};
      break;

    default:
      return false;
    }

    return true;
  }
};
#endif
