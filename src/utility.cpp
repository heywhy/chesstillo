#include <cstddef>
#include <cstdint>
#include <cstring>

#include <chesstillo/board.hpp>
#include <chesstillo/types.hpp>
#include <chesstillo/utility.hpp>

bool PieceToChar(char *c, Piece piece) {
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

bool PieceToChar(char *c, Piece piece, Color color) {
  if (!PieceToChar(c, piece)) {
    return false;
  }

  *c = color == BLACK ? *c : toupper(*c);

  return true;
}

Move DeduceMove(Position &position, uint8_t from, uint8_t to) {
  Piece piece;
  Piece captured;

  position.PieceAt(&piece, from);

  Move move(from, to, piece);

  if (position.PieceAt(&captured, to)) {
    move.captured = captured;
    move.Set(CAPTURE);
  }

  if (BITBOARD_FOR_SQUARE(to) & position.EnPassantSquare()) {
    move.Set(EN_PASSANT);
  }

  if (piece == KING && (from == e1 || from == e8) && (to == c1 || to == c8)) {
    move.Set(Flag::CASTLE_LEFT);
  }

  if (piece == KING && (from == e1 || from == e8) && (to == g1 || to == g8)) {
    move.Set(Flag::CASTLE_RIGHT);
  }

  return move;
}

bool ToString(char *buf, Move const &move) {
  Coord to;
  Coord from;

  if (!CoordForSquare(&from, move.from) || !CoordForSquare(&to, move.to)) {
    return false;
  }

  buf[0] = from.file;
  buf[1] = 48 + from.rank;

  buf[2] = to.file;
  buf[3] = 48 + to.rank;

  if (move.Is(PROMOTION)) {
    char piece;
    PieceToChar(&piece, move.promoted);

    buf[4] = piece;
    buf[5] = '\0';
  } else {
    buf[4] = '\0';
  }

  return true;
}
