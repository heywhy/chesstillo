#ifndef MOVE_GEN_HPP
#define MOVE_GEN_HPP

#include <tuple>
#include <vector>

#include <chesstillo/board.hpp>
#include <chesstillo/constants.hpp>
#include <chesstillo/position.hpp>
#include <chesstillo/types.hpp>

#define MOVE_NORTH(bitboard) bitboard << 8
#define MOVE_SOUTH(bitboard) bitboard >> 8
#define MOVE_EAST(bitboard) (bitboard << 1) & ~kAFile
#define MOVE_NORTH_EAST(bitboard) ((bitboard << 9) & ~kAFile)
#define MOVE_SOUTH_EAST(bitboard) (bitboard >> 7) & ~kAFile
#define MOVE_WEST(bitboard) (bitboard >> 1) & ~kHFile
#define MOVE_SOUTH_WEST(bitboard) (bitboard >> 9) & ~kHFile
#define MOVE_NORTH_WEST(bitboard) ((bitboard << 7) & ~kHFile)

#define MOVE_NORTH_NORTH_EAST(bitboard) (bitboard << 17) & ~kAFile
#define MOVE_NORTH_EAST_EAST(bitboard) (bitboard << 10) & ~(kAFile | kBFile)
#define MOVE_SOUTH_EAST_EAST(bitboard) (bitboard >> 6) & ~(kAFile | kBFile)
#define MOVE_SOUTH_SOUTH_EAST(bitboard) (bitboard >> 15) & ~kAFile
#define MOVE_NORTH_NORTH_WEST(bitboard) (bitboard << 15) & ~kHFile
#define MOVE_NORTH_WEST_WEST(bitboard) (bitboard << 6) & ~(kGFile | kHFile)
#define MOVE_SOUTH_WEST_WEST(bitboard) (bitboard >> 10) & ~(kGFile | kHFile)
#define MOVE_SOUTH_SOUTH_WEST(bitboard) (bitboard >> 17) & ~kHFile

#define KNIGHT_ATTACKS(bitboard)                                               \
  (MOVE_NORTH_NORTH_EAST(bitboard) | MOVE_NORTH_EAST_EAST(bitboard) |          \
   MOVE_SOUTH_EAST_EAST(bitboard) | MOVE_SOUTH_SOUTH_EAST(bitboard) |          \
   MOVE_NORTH_NORTH_WEST(bitboard) | MOVE_NORTH_WEST_WEST(bitboard) |          \
   MOVE_SOUTH_WEST_WEST(bitboard) | MOVE_SOUTH_SOUTH_WEST(bitboard))

#define KING_ATTACKS(bitboard)                                                 \
  (MOVE_NORTH(bitboard) | MOVE_EAST(bitboard) | MOVE_SOUTH(bitboard) |         \
   MOVE_WEST(bitboard) | MOVE_NORTH_EAST(bitboard) |                           \
   MOVE_NORTH_WEST(bitboard) | MOVE_SOUTH_EAST(bitboard) |                     \
   MOVE_SOUTH_WEST(bitboard))

#define LOOP_INDEX static_cast<int>(_loop_i)
#define BITLOOP(X)                                                             \
  for (Bitboard _x_copy = X, _loop_i = BIT_INDEX(_x_copy); _x_copy;            \
       _x_copy ^= BITBOARD_FOR_SQUARE(LOOP_INDEX),                             \
                _loop_i = BIT_INDEX(_x_copy))

Bitboard CheckMask(Position &position);
std::tuple<Bitboard, Bitboard> PinMask(Position &position);

std::vector<Move> GenerateMoves(Position &position);

void AddMovesToList(std::vector<Move> &moves, int from, Bitboard targets,
                    Piece piece, Piece *mailbox, Bitboard enemy_bb);

constexpr Bitboard RankMask(int square) { return kRank1 << (square & 56); }

constexpr Bitboard FileMask(int square) { return kAFile << (square & 7); }

constexpr Bitboard DiagonalMask(int square) {
  int diagonal = (square & 7) - (square >> 3);

  return diagonal >= 0 ? kA1H8Diagonal >> diagonal * 8
                       : kA1H8Diagonal << -diagonal * 8;
}

constexpr Bitboard AntiDiagonalMask(int square) {
  int diagonal = 7 - (square & 7) - (square >> 3);

  return diagonal >= 0 ? kH1A8Diagonal >> diagonal * 8
                       : kH1A8Diagonal << -diagonal * 8;
}

constexpr Bitboard SquaresInBetween(unsigned int from, unsigned int to) {
  Bitboard const M = -1;
  Bitboard btwn, line, rank, file;

  to = to >= 64 ? 63 : to;
  from = from >= 64 ? 63 : from;

  btwn = (M << from) ^ (M << to);
  file = (to & 7) - (from & 7);
  rank = ((to | 7) - from) >> 3;
  line = ((file & 7) - 1) & kA2A7;

  line += 2 * (((rank & 7) - 1) >> 58);
  line += (((rank - file) & 15) - 1) & kB2G7;
  line += (((rank + file) & 15) - 1) & kH1B7;
  line *= btwn & -btwn;

  return line & btwn;
}

constexpr Bitboard BishopXRayAttacks(Bitboard attacks, Bitboard occupied_sqs,
                                     Bitboard blockers, unsigned int square) {
  return attacks ^
         kSlidingAttacks.Bishop(occupied_sqs ^ (blockers & attacks), square);
}

constexpr Bitboard RookXRayAttacks(Bitboard attacks, Bitboard occupied_sqs,
                                   Bitboard blockers, unsigned int square) {
  return attacks ^
         kSlidingAttacks.Rook(occupied_sqs ^ (blockers & attacks), square);
}

template <enum Color side> constexpr Bitboard PushPawn(Bitboard b) {
  if constexpr (side == WHITE) {
    return MOVE_NORTH(b);
  }

  if constexpr (side == BLACK) {
    return MOVE_SOUTH(b);
  }

  return kEmpty;
}

template <enum Color side>
constexpr Bitboard DoublePushPawn(Bitboard b, Bitboard empty) {
  if constexpr (side == WHITE) {
    b &= kRank2;
    b = MOVE_NORTH(b) & empty;

    return MOVE_NORTH(b) & empty;
  }

  if constexpr (side == BLACK) {
    b &= kRank7;
    b = MOVE_SOUTH(b) & empty;

    return MOVE_SOUTH(b) & empty;
  }
}

enum Compass { EAST, WEST };

template <enum Color side, enum Compass compass>
constexpr Bitboard PawnTargets(Bitboard b) {
  if constexpr (side == WHITE && compass == WEST) {
    return MOVE_NORTH_WEST(b);
  }

  if constexpr (side == WHITE && compass == EAST) {
    return MOVE_NORTH_EAST(b);
  }

  if constexpr (side == BLACK && compass == WEST) {
    return MOVE_SOUTH_WEST(b);
  }

  if constexpr (side == BLACK && compass == EAST) {
    return MOVE_SOUTH_EAST(b);
  }
}
#endif
