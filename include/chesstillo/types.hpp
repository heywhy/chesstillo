#ifndef TYPES_HPP
#define TYPES_HPP

#include <cctype>
#include <cstdint>
#include <vector>

#define COLOR 2
#define PIECES 6
#define MAX_SCORE INT_MAX
#define MIN_SCORE -INT_MAX
#define SCORE_INF INT_MAX

struct Move;
struct SplitPoint;

typedef std::uint64_t Bitboard;
typedef std::vector<Move> Moves;

enum Color : uint8_t { WHITE, BLACK };
enum CastleDir : uint8_t { LEFT, RIGHT };
enum Piece : uint8_t { ROOK, BISHOP, KNIGHT, KING, QUEEN, PAWN, NONE };
// exact, lower bound, upper bound
enum NodeType : uint8_t { PV, CUT, ALL };
enum Flag : uint8_t {
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
  (static_cast<uint8_t>(1) << (color == WHITE ? LEFT + color                   \
                                              : LEFT + color + 1))
#define CASTLE_RIGHT(color)                                                    \
  (static_cast<uint8_t>(1) << (color == WHITE ? RIGHT + color                  \
                                              : RIGHT + color + 1))
#define CASTLE_LEFT_WHITE (LEFT + WHITE)
#define CASTLE_LEFT_BLACK (LEFT + BLACK + 1)
#define CASTLE_RIGHT_WHITE (RIGHT + WHITE)
#define CASTLE_RIGHT_BLACK (RIGHT + BLACK + 1)

// clang-format off
enum ESquare : uint8_t {
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

struct Move {
  uint8_t from;
  uint8_t to;
  Piece piece;
  uint8_t flags;
  Piece captured;
  Piece promoted;

  int cost = 0;
  int score = 0;

  Move() {}

  Move(uint8_t from, uint8_t to, Piece piece)
      : from(from), to(to), piece(piece), flags(0) {}

  Move(uint8_t from, uint8_t to, Piece piece, uint8_t flags)
      : from(from), to(to), piece(piece), flags(flags) {}

  void Set(uint8_t flag) { flags |= 1U << flag; }

  bool Is(uint8_t flag) const { return flags & (1U << flag); }

  static Move None() { return {0, 0, NONE}; }

  friend inline bool operator==(const Move &lhs, const Move &rhs) {
    return lhs.from == rhs.from && lhs.to == rhs.to && lhs.piece == rhs.piece;
  }
};
#endif
