#ifndef TYPES_HPP
#define TYPES_HPP

#include <cctype>
#include <cstdint>

typedef std::uint64_t Bitboard;

enum Color : std::uint8_t { WHITE, BLACK };
enum CastleDir : std::uint8_t { LEFT, RIGHT };
enum Piece : std::uint8_t { ROOK, BISHOP, KNIGHT, KING, QUEEN, PAWN };
enum Flag : std::uint8_t {
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

#define CASTLE_LEFT(color)                                                     \
  (static_cast<std::uint8_t>(1)                                                \
   << (color == WHITE ? LEFT + color : LEFT + color + 1))
#define CASTLE_RIGHT(color)                                                    \
  (static_cast<std::uint8_t>(1)                                                \
   << (color == WHITE ? RIGHT + color : RIGHT + color + 1))
#define CASTLE_LEFT_WHITE (LEFT + WHITE)
#define CASTLE_LEFT_BLACK (LEFT + BLACK + 1)
#define CASTLE_RIGHT_WHITE (RIGHT + WHITE)
#define CASTLE_RIGHT_BLACK (RIGHT + BLACK + 1)

// clang-format off
enum ESquare : unsigned int {
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

// TODO: Maybe find a way to avoid move copying
struct Move {
  unsigned int from;
  unsigned int to;
  Piece piece;
  unsigned int flags;
  Piece captured;
  Piece promoted;

  Move(unsigned int from, unsigned int to, Piece piece)
      : from(from), to(to), piece(piece), flags(0) {}

  Move(unsigned int from, unsigned int to, Piece piece, unsigned int flags)
      : from(from), to(to), piece(piece), flags(flags) {}

  void Set(unsigned int flag) { flags |= 1U << flag; }

  bool Is(unsigned int flag) const { return flags & (1U << flag); }

  friend inline bool operator==(const Move &lhs, const Move &rhs) {
    return lhs.from == rhs.from && lhs.to == rhs.to && lhs.piece == rhs.piece;
  }
};

#endif
