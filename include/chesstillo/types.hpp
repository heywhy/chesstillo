#ifndef TYPES_HPP
#define TYPES_HPP

#include <cctype>
#include <cstdint>

#define CHECK 0
#define CAPTURE 1
#define CHECKMATE 2
#define PROMOTION 3
#define EN_PASSANT 4
#define DISCOVERY 5
#define DOUBLE 6
#define CASTLE 7

typedef std::uint64_t Bitboard;

enum Color : unsigned int { WHITE, BLACK };

enum Piece : unsigned int { ROOK, BISHOP, KNIGHT, KING, QUEEN, PAWN };

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
  Bitboard ep_target;
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
