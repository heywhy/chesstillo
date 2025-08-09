#include <cstddef>
#include <cstring>

#include "engine/board.hpp"
#include "engine/move.hpp"
#include "engine/square.hpp"
#include "engine/types.hpp"
#include "engine/utils.hpp"

namespace engine {

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

  *c = color == BLACK ? *c : std::toupper(*c);

  return true;
}

Move DeduceMove(const Position &position, int from, int to) {
  Piece piece;
  Piece captured;

  position.PieceAt(&piece, from);

  Move move(from, to, piece);

  if (position.PieceAt(&captured, to)) {
    move.captured = captured;
    move.Set(move::CAPTURE);
  }

  if (square::BB(to) & position.EnPassantSquare()) {
    move.Set(move::EN_PASSANT);
  }

  if (piece == KING && (from == e1 || from == e8) && (to == c1 || to == c8)) {
    move.Set(move::CASTLE_QUEEN_SIDE);
  }

  if (piece == KING && (from == e1 || from == e8) && (to == g1 || to == g8)) {
    move.Set(move::CASTLE_KING_SIDE);
  }

  return move;
}

bool ToString(char *buf, const Move &move) {
  Coord to;
  Coord from;

  if (!CoordForSquare(&from, move.from) || !CoordForSquare(&to, move.to)) {
    return false;
  }

  buf[0] = from.file;
  buf[1] = 49 + from.rank;

  buf[2] = to.file;
  buf[3] = 49 + to.rank;

  if (move.Is(move::PROMOTION)) {
    char piece;
    PieceToChar(&piece, move.promoted);

    buf[4] = piece;
    buf[5] = '\0';
  } else {
    buf[4] = '\0';
  }

  return true;
}

}  // namespace engine
