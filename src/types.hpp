#include <cstdint>

#ifndef TYPES_HPP

#define TYPES_HPP

#define CAPTURE 1
#define PROMOTION 2

typedef std::uint64_t Bitboard;

enum Color { WHITE, BLACK };

enum Piece { ROOK, KNIGHT, BISHOP, KING, QUEEN, PAWN };

struct Move {
  Bitboard from;
  Bitboard to;
  Color color;
  Piece piece;
  std::uint8_t flags;

  Move(Bitboard from, Bitboard to, Color color, Piece piece)
      : Move(from, to, color, piece, 0) {}

  Move(Bitboard from, Bitboard to, Color color, Piece piece, std::uint8_t flags)
      : from(from), to(to), color(color), piece(piece), flags(flags) {}

  bool IsCapture() { return flags & (1U << CAPTURE); }
  bool IsPromotion() { return flags & (1U << PROMOTION); }
};

#endif
