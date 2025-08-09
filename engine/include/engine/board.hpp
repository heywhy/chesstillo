#ifndef ENGINE_BOARD_HPP
#define ENGINE_BOARD_HPP

#include <cstdint>

#include "types.hpp"

#define OPP(turn) static_cast<Color>(turn ^ 1)

namespace engine {

struct Coord {
  char file;
  std::uint8_t rank;
};

bool CoordForSquare(Coord *coord, int square);

class Board {
 public:
  PieceList pieces[2];
  Bitboard occupied_sqs;

  Board();

  void Reset();
  void UpdateOccupiedSqs();
};

}  // namespace engine

#endif
