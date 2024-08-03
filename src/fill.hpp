#ifndef FILL_HPP
#define FILL_HPP

#include <chesstillo/move.hpp>
#include <chesstillo/types.hpp>

#define FILE_ATTACKS(bb, empty)                                                \
  MOVE_SOUTH(SouthOccluded(bb, empty)) | MOVE_NORTH(NorthOccluded(bb, empty))

#define RANK_ATTACKS(bb, empty)                                                \
  MOVE_EAST(EastOccluded(bb, empty)) | MOVE_WEST(WestOccluded(bb, empty))

#define DIAGONAL_ATTACKS(bb, empty)                                            \
  MOVE_NORTH_EAST(NorthEastOccluded(bb, empty)) |                              \
      MOVE_SOUTH_WEST(SouthWestOccluded(bb, empty))

#define ANTI_DIAGONAL_ATTACKS(bb, empty)                                       \
  MOVE_NORTH_WEST(NorthWestOccluded(bb, empty)) |                              \
      MOVE_SOUTH_EAST(SouthEastOccluded(bb, empty))

#define BISHOP_ATTACKS(bb, empty)                                              \
  ANTI_DIAGONAL_ATTACKS(bb, empty) | DIAGONAL_ATTACKS(bb, empty)

#define ROOK_ATTACKS(bb, empty)                                                \
  FILE_ATTACKS(bb, empty) | RANK_ATTACKS(bb, empty)

#define QUEEN_ATTACKS(bb, empty)                                               \
  (FILE_ATTACKS(bb, empty) | RANK_ATTACKS(bb, empty)) ^                        \
      (DIAGONAL_ATTACKS(bb, empty) | ANTI_DIAGONAL_ATTACKS(bb, empty))

Bitboard SouthOccluded(Bitboard bb, Bitboard pro);
Bitboard NorthOccluded(Bitboard bb, Bitboard pro);
Bitboard EastOccluded(Bitboard bb, Bitboard pro);
Bitboard WestOccluded(Bitboard bb, Bitboard pro);

Bitboard NorthEastOccluded(Bitboard bb, Bitboard pro);
Bitboard NorthWestOccluded(Bitboard bb, Bitboard pro);
Bitboard SouthWestOccluded(Bitboard bb, Bitboard pro);
Bitboard SouthEastOccluded(Bitboard bb, Bitboard pro);
#endif
