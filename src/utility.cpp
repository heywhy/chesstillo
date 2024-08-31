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
