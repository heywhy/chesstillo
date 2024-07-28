#ifndef GUI_SQUARE_HPP
#define GUI_SQUARE_HPP

#include <chesstillo/board.hpp>
#include <chesstillo/constants.hpp>
#include <chesstillo/types.hpp>
#include <ftxui/component/component_base.hpp>
#include <ftxui/component/event.hpp>
#include <ftxui/screen/color.hpp>
#include <string>
#include <tuple>

#include "fonts.hpp"
#include "theme.hpp"
#include "utils.hpp"

class Square : public ftxui::ComponentBase {
public:
  Square(const Theme *theme, OnSelectSquare *observer, std::uint8_t index,
         std::int8_t *selected)
      : theme_(theme), observer_(observer), index_(index), selected_(selected),
        piece_('\0'), bitboard(BITBOARD_FOR_SQUARE(index_)) {}

  void SetPiece(char piece) { piece_ = piece; }
  bool IsEmpty() { return piece_ == '\0'; }

  bool Font(std::tuple<std::string, ftxui::Color> *result) {
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

  ftxui::Element Render() override {
    ftxui::Elements elements;
    std::tuple<std::string, ftxui::Color> piece;

    if (Font(&piece)) {
      ftxui::Element content =
          ftxui::text(std::get<0>(piece)) | ftxui::color(std::get<1>(piece));

      elements.push_back(content);
    }

    ftxui::Element square = ftxui::vbox(elements) | ftxui::center;

    square |= ftxui::size(ftxui::WIDTH, ftxui::EQUAL, 7) |
              ftxui::size(ftxui::HEIGHT, ftxui::EQUAL, 3);

    if (*selected_ != index_) {
      ftxui::Color color =
          bitboard & kDarkSquares ? theme_->dark_square : theme_->light_square;

      square |= ftxui::bgcolor(color);
    } else {
      square |= ftxui::bgcolor(theme_->selected_square);
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
  const Theme *theme_;
  OnSelectSquare *observer_;
  std::uint8_t index_;
  std::int8_t *selected_;
  char piece_;

public:
  Bitboard const bitboard;
};

#endif
