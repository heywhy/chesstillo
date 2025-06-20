#include <cstdint>

#include <engine/board.hpp>
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

void Board::Reset() {
  occupied_sqs_ = kEmpty;

  pieces_[WHITE][KING] = pieces_[WHITE][QUEEN] = pieces_[WHITE][ROOK] =
      pieces_[WHITE][KNIGHT] = pieces_[WHITE][BISHOP] = pieces_[WHITE][PAWN] =
          kEmpty;

  pieces_[BLACK][KING] = pieces_[BLACK][QUEEN] = pieces_[BLACK][ROOK] =
      pieces_[BLACK][KNIGHT] = pieces_[BLACK][BISHOP] = pieces_[BLACK][PAWN] =
          kEmpty;
}

}  // namespace engine
