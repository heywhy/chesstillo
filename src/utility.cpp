#include <cstddef>
#include <cstring>

#include <chesstillo/board.hpp>
#include <chesstillo/types.hpp>
#include <chesstillo/utility.hpp>

bool PieceToChar(Piece piece, Color color, char *c) {
  switch (piece) {
  case ROOK:
    *c = 'r';
    break;
  case KNIGHT:
    *c = 'n';
    break;
  case BISHOP:
    *c = 'b';
    break;
  case QUEEN:
    *c = 'q';
    break;
  case KING:
    *c = 'k';
    break;
  case PAWN:
    *c = 'p';
    break;
  default:
    return false;
  }

  *c = color == BLACK ? *c : toupper(*c);

  return true;
}

bool MoveToString(Move const &move, char *text) {
  Coord to;
  Coord from;

  if (!CoordFromBitboard(move.from, &from) ||
      !CoordFromBitboard(move.to, &to)) {
    return false;
  }

  int i = 0;
  char piece;
  char buffer[6];

  if (move.piece != PAWN && PieceToChar(move.piece, move.color, &piece)) {
    buffer[i++] = piece;

    if (move.Is(CAPTURE)) {
      buffer[i++] = 'x';
    }

    buffer[i++] = to.file;
  } else if (move.Is(CAPTURE)) {
    buffer[i++] = from.file;
    buffer[i++] = 'x';
    buffer[i++] = to.file;
  } else {
    buffer[i++] = to.file;
  }

  buffer[i++] = 48 + to.rank;

  if (move.Is(CHECK)) {
    buffer[i++] = '+';
  }

  buffer[i++] = '\0';

  std::strcpy(text, buffer);

  return true;
}

std::size_t Split(Bitboard bb, Bitboard *const out) {
  std::size_t size = 0;

  Bitboard lsb = bb & -bb;
  Bitboard rest = bb ^ lsb;

  while (lsb) {
    out[size++] = lsb;

    lsb = rest & -rest;
    rest ^= lsb;
  }

  return size;
}
