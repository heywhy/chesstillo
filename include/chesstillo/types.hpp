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

typedef std::uint64_t Bitboard;

enum Color { WHITE, BLACK };

enum Piece { ROOK, KNIGHT, BISHOP, KING, QUEEN, PAWN };

// TODO: Maybe find a way to avoid move copying
struct Move {
  Bitboard from;
  Bitboard to;
  Bitboard ep_target;
  Piece captured;
  Color color;
  Piece piece;
  unsigned int flags = 0;

  Move(Bitboard from, Bitboard to, Color color, Piece piece)
      : Move(from, to, color, piece, 0) {}

  Move(Bitboard from, Bitboard to, Color color, Piece piece, unsigned int flags)
      : from(from), to(to), color(color), piece(piece), flags(flags) {}

  void Set(unsigned int flag) { flags |= 1U << flag; }

  bool Is(unsigned int flag) const { return flags & (1U << flag); }

  bool operator==(Move &move) const {
    return from == move.from && to == move.to && piece == move.piece &&
           color == move.color;
  }
};

bool MoveToString(Move const &move, char *text);
bool PieceToChar(Piece piece, Color color, char *c);

#endif
