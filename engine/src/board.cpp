#include <cstdint>

#include <engine/board.hpp>
#include <engine/constants.hpp>
#include <engine/square.hpp>
#include <engine/types.hpp>
#include <engine/utility.hpp>

namespace engine {

bool CoordForSquare(Coord *coord, int square) {
  if (square >= 0 && square < 64) {
    int file = square::File(square);
    int rank = square::Rank(square);

    *coord = {static_cast<char>(file + 97), static_cast<std::uint8_t>(rank)};

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

void Board::UpdateOccupiedSqs() {
  occupied_sqs =
      square::Occupancy(pieces[WHITE]) | square::Occupancy(pieces[BLACK]);
}

}  // namespace engine
