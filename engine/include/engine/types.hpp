#ifndef TYPES_HPP
#define TYPES_HPP

#include <array>
#include <climits>
#include <cstdint>

#define COLOR 2
#define PIECES 6
#define MAX_SCORE INT_MAX
#define MIN_SCORE -INT_MAX
#define SCORE_INF INT_MAX

#define CASTLE_LEFT(color)           \
  (static_cast<std::uint_fast8_t>(1) \
   << (color == WHITE ? LEFT + color : LEFT + color + 1))
#define CASTLE_RIGHT(color)          \
  (static_cast<std::uint_fast8_t>(1) \
   << (color == WHITE ? RIGHT + color : RIGHT + color + 1))
#define CASTLE_LEFT_WHITE (LEFT + WHITE)
#define CASTLE_LEFT_BLACK (LEFT + BLACK + 1)
#define CASTLE_RIGHT_WHITE (RIGHT + WHITE)
#define CASTLE_RIGHT_BLACK (RIGHT + BLACK + 1)

namespace engine {

using Bitboard = std::uint64_t;
using PieceList = std::array<Bitboard, 6>;

enum Color : std::uint_fast8_t { WHITE, BLACK };

enum CastleDir : std::uint_fast8_t { LEFT, RIGHT };

enum Piece : std::uint_fast8_t {
  ROOK,
  BISHOP,
  KNIGHT,
  KING,
  QUEEN,
  PAWN,
  NONE
};

// exact, lower bound, upper bound
enum NodeType : std::uint_fast8_t { PV, CUT, ALL };

enum Flag : std::uint_fast8_t {
  CHECK,
  CAPTURE,
  EN_PASSANT,
  CASTLE_RIGHT,
  CASTLE_LEFT,
  CHECKMATE,
  PROMOTION,
  // DISCOVERY,
  // DOUBLE
};

// clang-format off
enum ESquare : std::uint_fast8_t {
  a1, b1, c1, d1, e1, f1, g1, h1,
  a2, b2, c2, d2, e2, f2, g2, h2,
  a3, b3, c3, d3, e3, f3, g3, h3,
  a4, b4, c4, d4, e4, f4, g4, h4,
  a5, b5, c5, d5, e5, f5, g5, h5,
  a6, b6, c6, d6, e6, f6, g6, h6,
  a7, b7, c7, d7, e7, f7, g7, h7,
  a8, b8, c8, d8, e8, f8, g8, h8
};
// clang-format on

}  // namespace engine

#endif
