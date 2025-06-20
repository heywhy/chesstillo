#ifndef BOARD_HPP
#define BOARD_HPP

#include <cstdint>

#include "types.hpp"

#define OPP(turn) static_cast<Color>(turn ^ 1)
#define TO_SQUARE(file, rank) 8 * rank + file
#define RANK(square) ((square / 8) + 1)
#define FILE(square) (square % 8)
#define BIT_INDEX(x) std::countr_zero(x)
#define BIT_INDEX_MSB(x) (63 - std::countl_zero(x))
#define BITBOARD_FOR_SQUARE(index) static_cast<Bitboard>(1) << index
#define BOARD_OCCUPANCY(pieces)                                     \
  (pieces[KING] | pieces[QUEEN] | pieces[BISHOP] | pieces[KNIGHT] | \
   pieces[ROOK] | pieces[PAWN])

namespace engine {

struct Coord {
  char file;
  std::uint8_t rank;
};

bool CoordForSquare(Coord *coord, std::uint8_t square);

class Board {
 public:
  // pawn, rook, knight, bishop, queen, king
  Bitboard pieces[2][6];
  Bitboard occupied_sqs;

  Board();

  void Reset();

  void UpdateOccupiedSqs() {
    occupied_sqs =
        BOARD_OCCUPANCY(pieces[WHITE]) | BOARD_OCCUPANCY(pieces[BLACK]);
  }
};

}  // namespace engine

#endif
