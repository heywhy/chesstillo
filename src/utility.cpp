#include <cstring>

#include "chesstillo/board.hpp"
#include "chesstillo/types.hpp"

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
  char buffer[5];

  if (move.piece != PAWN && PieceToChar(move.piece, move.color, &piece)) {
    buffer[i++] = piece;

    if (move.IsCapture()) {
      buffer[i++] = 'x';
    }

    buffer[i++] = to.file;
  } else if (move.IsCapture()) {
    buffer[i++] = from.file;
    buffer[i++] = 'x';
    buffer[i++] = to.file;
  } else {
    buffer[i++] = to.file;
  }

  buffer[i++] = 48 + to.rank;
  buffer[i++] = '\0';

  std::strcpy(text, buffer);

  return true;
}
