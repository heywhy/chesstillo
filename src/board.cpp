#include <algorithm>
#include <bit>

#include "board.hpp"
#include "move.hpp"

static const char kFiles[] = {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h'};

Bitboard BitboardForSquare(int file, int rank) {
  // int square = 8 * file + rank;
  // int square = 8 * (rank - 1) + file;
  int square = 8 * rank + file;

  return 1ULL << square;
}

Coord CoordFromBitboard(Bitboard square) {
  if (square) {
    int index = std::popcount((square & -square) - 1);
    int file = index % 8;
    int rank = index / 8;

    return {kFiles[file], rank + 1};
  }

  return {'\0', -1};
}

Bitboard BitboardForSquare(char file, int rank) {
  int n = sizeof(kFiles) / sizeof(char);

  const char *element = std::find(&kFiles[0], kFiles + n, file);
  int index = element - kFiles;

  return BitboardForSquare(index, rank - 1);
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
  en_passant_sq_ = EMPTY;
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
