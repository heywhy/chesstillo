#include "ftxui/dom/elements.hpp"
#include <chesstillo/board.hpp>
#include <ftxui/component/component.hpp>
#include <ftxui/component/component_base.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/dom/flexbox_config.hpp>
#include <ftxui/dom/node.hpp>
#include <ftxui/screen/color.hpp>
#include <string>

Bitboard const kSquare = static_cast<Bitboard>(1);

class ChessBoard : public ftxui::ComponentBase {
public:
  ChessBoard(Board board) : board_(board) {}

  ftxui::Element Render() {
    ftxui::FlexboxConfig config;

    config.Set(ftxui::FlexboxConfig::Direction::Column);
    config.Set(ftxui::FlexboxConfig::AlignContent::Center);
    config.Set(ftxui::FlexboxConfig::JustifyContent::Center);

    ftxui::Elements board;

    for (int i = 7; i >= 0; i--) {
      ftxui::Elements row;

      for (int j = 0; j < 8; j++) {
        char piece = board_.PieceAtSquare(BitboardForSquare(j, i));

        ftxui::Elements e;

        if (piece != '\0') {
          std::string p(1, piece);

          e.push_back({ftxui::text(p)});
        }

        ftxui::Element square = ftxui::dbox(e | ftxui::center) |
                                ftxui::size(ftxui::WIDTH, ftxui::EQUAL, 4) |
                                ftxui::size(ftxui::HEIGHT, ftxui::EQUAL, 2);

        if ((i + j) % 2 == 0) {
          square |= ftxui::bgcolor(ftxui::Color::Black) |
                    ftxui::color(ftxui::Color::White);
        } else {
          square |= ftxui::bgcolor(ftxui::Color::White) |
                    ftxui::color(ftxui::Color::Black);
        }

        row.push_back(square);
      }

      board.push_back(ftxui::hbox(row));
    }

    ftxui::Element box = ftxui::vbox(board);

    return ftxui::flexbox({box}, config) | ftxui::border;
  }

private:
  Board board_;
};
