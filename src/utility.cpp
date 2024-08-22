#include <cstddef>
#include <cstring>

#include <chesstillo/board.hpp>
#include <chesstillo/types.hpp>
#include <chesstillo/utility.hpp>

bool PieceToChar(Piece piece, char *c) {
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

  return true;
}

bool PieceToChar(Piece piece, Color color, char *c) {
  if (!PieceToChar(piece, c)) {
    return false;
  }

  *c = color == BLACK ? *c : toupper(*c);

  return true;
}

bool MoveToString(Move const &move, Color turn, char *text) {
  Coord to;
  Coord from;

  if (!CoordForSquare(&from, move.from) || !CoordForSquare(&to, move.to)) {
    return false;
  }

  int i = 0;
  char piece;
  char buffer[6];

  if (move.piece != PAWN && PieceToChar(move.piece, turn, &piece)) {
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

  if (move.Is(CHECKMATE)) {
    buffer[i - 1] = '#';
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

Move DeduceMove(Position &position, unsigned int from, unsigned int to) {
  Piece piece;
  Piece captured;

  position.PieceAtSquare(from, &piece);

  Move move(from, to, piece);

  if (position.PieceAtSquare(to, &captured)) {
    move.captured = captured;
    move.Set(CAPTURE);
  }

  if (BITBOARD_FOR_SQUARE(to) & position.en_passant_sq_) {
    move.ep_target = position.EnPassantTarget();

    move.Set(EN_PASSANT);
  }

  return move;
}
