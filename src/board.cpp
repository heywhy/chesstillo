#include "board.hpp"
#include "move.hpp"

Bitboard BitboardForSquare(File file, int rank) {
  int square = 8 * file + rank;

  return 1ULL << square;
}

void Board::Reset() {
  for (int i = 0; i < 6; i++) {
    w_pieces_[i] = EMPTY;
  }

  for (int i = 0; i < 6; i++) {
    b_pieces_[i] = EMPTY;
  }

  turn_ = WHITE;
  castling_rights_ = 0;
}

bool Board::Endgame() { return false; }

void Board::ApplyMove(Move move) {
  Bitboard *piece =
      move.color == WHITE ? &w_pieces_[move.piece] : &b_pieces_[move.piece];

  *piece ^= move.from ^ move.to;

  moves.push_front(move);

  ComputeOccupied();
}

void Board::Print() {}
