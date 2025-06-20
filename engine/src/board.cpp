#include <cstdint>

#include <engine/board.hpp>
#include <engine/constants.hpp>
#include <engine/types.hpp>
#include <engine/utility.hpp>

namespace engine {

bool CoordForSquare(Coord *coord, uint8_t square) {
  if (square >= 0 && square < 64) {
    uint8_t file = FILE(square);
    uint8_t rank = RANK(square);

    *coord = {static_cast<char>(file + 97), static_cast<uint8_t>(rank)};

    return true;
  }

  return false;
}

Board::Board() { Reset(); }

void Board::Reset() {
  occupied_sqs = kEmpty;

  for (int i = 0; i < 6; i++) {
    pieces[0][i] = kEmpty;
    pieces[1][i] = kEmpty;
  }
}

}  // namespace engine
