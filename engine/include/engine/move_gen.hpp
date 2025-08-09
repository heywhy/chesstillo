#ifndef ENGINE_MOVE_GEN_HPP
#define ENGINE_MOVE_GEN_HPP

#include <utility>

#include "constants.hpp"
#include "move.hpp"
#include "position.hpp"
#include "types.hpp"

// The max number of squares any piece can travel to is that of the queen
#define MAX_PIECE_MOVES 27
// Assuming the maximum number of any piece type asides king &
// pawns is 10
#define MAX_PIECE_NUMBER 10
#define MAX_MOVES_BUFFER_SIZE 256
#define IN_CHECK(mask) mask != kUniverse

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

#define KNIGHT_ATTACKS(bitboard)                                      \
  (MOVE_NORTH_NORTH_EAST(bitboard) | MOVE_NORTH_EAST_EAST(bitboard) | \
   MOVE_SOUTH_EAST_EAST(bitboard) | MOVE_SOUTH_SOUTH_EAST(bitboard) | \
   MOVE_NORTH_NORTH_WEST(bitboard) | MOVE_NORTH_WEST_WEST(bitboard) | \
   MOVE_SOUTH_WEST_WEST(bitboard) | MOVE_SOUTH_SOUTH_WEST(bitboard))

#define KING_ATTACKS(bitboard)                                         \
  (MOVE_NORTH(bitboard) | MOVE_EAST(bitboard) | MOVE_SOUTH(bitboard) | \
   MOVE_WEST(bitboard) | MOVE_NORTH_EAST(bitboard) |                   \
   MOVE_NORTH_WEST(bitboard) | MOVE_SOUTH_EAST(bitboard) |             \
   MOVE_SOUTH_WEST(bitboard))

#define LOOP_INDEX static_cast<int>(_loop_i)
#define BITLOOP(X)                                                      \
  for (Bitboard _x_copy = X, _loop_i = square::Index(_x_copy); _x_copy; \
       _x_copy ^= square::BB(LOOP_INDEX), _loop_i = square::Index(_x_copy))

namespace engine {

Bitboard CheckMask(const Position &position);
std::pair<Bitboard, Bitboard> PinMask(const Position &position);

MoveList GenerateMoves(const Position &position);

void AddMovesToList(MoveList &moves, int from, Bitboard targets, Piece piece,
                    const Mailbox &mailbox, Bitboard enemy_bb);

constexpr Bitboard BishopXRayAttacks(Bitboard attacks, Bitboard occupied_sqs,
                                     Bitboard blockers, int square) {
  return attacks ^
         kSlidingAttacks.Bishop(occupied_sqs ^ (blockers & attacks), square);
}

constexpr Bitboard RookXRayAttacks(Bitboard attacks, Bitboard occupied_sqs,
                                   Bitboard blockers, int square) {
  return attacks ^
         kSlidingAttacks.Rook(occupied_sqs ^ (blockers & attacks), square);
}

template <enum Color side>
constexpr Bitboard PushPawn(Bitboard b) {
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
constexpr inline Bitboard PawnTargets(Bitboard b) {
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

template <enum Color side>
constexpr Bitboard PawnTargets(Bitboard b) {
  return PawnTargets<side, EAST>(b) | PawnTargets<side, WEST>(b);
}

}  // namespace engine

#endif
