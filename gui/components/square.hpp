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
#include "utils.hpp"

class Square : public ftxui::ComponentBase {
public:
  Square(OnSelectSquare *observer, std::uint8_t index, std::int8_t *selected)
      : observer_(observer), index_(index), selected_(selected), piece_('\0'),
        bitboard(BITBOARD_FOR_SQUARE(index_)) {
    color_ = bitboard & kDarkSquares
                 ? ftxui::Color(ftxui::Color::Palette256::DarkGoldenrod)
                 : ftxui::Color::White;
  }

  void SetPiece(char piece) { piece_ = piece; }
  bool IsEmpty() { return piece_ == '\0'; }

  bool Font(std::tuple<std::string, ftxui::Color> *result) {
    // TODO: These values should be picked from a selected board theme.
    ftxui::Color black = ftxui::Color::Black;
    ftxui::Color white = ftxui::Color::Grey93;

    switch (piece_) {
    case 'r':
      *result = {kRook, black};
      break;

    case 'R':
      *result = {kRook, white};
      break;

    case 'n':
      *result = {kKnight, black};
      break;

    case 'N':
      *result = {kKnight, white};
      break;

    case 'b':
      *result = {kBishop, black};
      break;

    case 'B':
      *result = {kBishop, white};
      break;

    case 'k':
      *result = {kKing, black};
      break;

    case 'K':
      *result = {kKing, white};
      break;

    case 'q':
      *result = {kQueen, black};
      break;

    case 'Q':
      *result = {kQueen, white};
      break;

    case 'p':
      *result = {kPawn, black};
      break;

    case 'P':
      *result = {kPawn, white};
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
      square |= ftxui::bgcolor(color_);
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
  ftxui::Color color_;

public:
  Bitboard const bitboard;
};

#endif
