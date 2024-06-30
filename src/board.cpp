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
    w_attacking_sqs_[i] = EMPTY;
  }

  for (int i = 0; i < 6; i++) {
    b_pieces_[i] = EMPTY;
    b_attacking_sqs_[i] = EMPTY;
  }

  turn_ = WHITE;
  castling_rights_ = 0;
  en_passant_sq_ = EMPTY;
}

bool Board::Endgame() { return false; }

void Board::ApplyMove(Move move) {
  if (turn_ != move.color || !IsValidMove(move))
    return;

  Bitboard sqs_occupied_by_opp =
      move.color == WHITE ? sqs_occupied_by_b_ : sqs_occupied_by_w_;

  Bitboard *piece =
      move.color == WHITE ? &w_pieces_[move.piece] : &b_pieces_[move.piece];

  *piece ^= move.from ^ move.to;

  if (sqs_occupied_by_opp & move.to) {
    Bitboard *pieces = move.color == WHITE ? b_pieces_ : w_pieces_;

    move.flags |= 1U << 1;

    for (int i = 0; i < 6; i++) {
      if (pieces[i] & move.to) {
        pieces[i] ^= move.to;
        break;
      }
    }
  }

  moves.push_front(move);

  turn_ = move.color == WHITE ? BLACK : WHITE;

  if (move.piece == PAWN || move.IsCapture()) {
    halfmove_clock_ = 0;
  } else {
    halfmove_clock_++;
  }

  if (move.color == BLACK)
    fullmove_counter_++;

  ComputeOccupiedSqs();
  ComputeAttackedSqs();
}

bool Board::IsValidMove(Move const &move) {
  Bitboard piece =
      move.color == WHITE ? w_pieces_[move.piece] : b_pieces_[move.piece];

  Bitboard attacking_sqs = move.color == WHITE ? w_attacking_sqs_[move.piece]
                                               : b_attacking_sqs_[move.piece];

  if (!(piece & move.from))
    return false;

  if (move.piece == PAWN)
    return IsValidPawnMove(*this, piece & move.from, move, attacking_sqs);

  return false;
}

void Board::ComputeAttackedSqs() {
  w_attacking_sqs_[PAWN] =
      (MOVE_NORTH_EAST(w_pieces_[PAWN]) ^ MOVE_NORTH_WEST(w_pieces_[PAWN])) |
      (MOVE_NORTH_EAST(w_pieces_[PAWN]) & MOVE_NORTH_WEST(w_pieces_[PAWN]));

  b_attacking_sqs_[PAWN] =
      (MOVE_SOUTH_EAST(b_pieces_[PAWN]) ^ MOVE_SOUTH_WEST(b_pieces_[PAWN])) |
      (MOVE_SOUTH_EAST(b_pieces_[PAWN]) & MOVE_SOUTH_WEST(b_pieces_[PAWN]));
}

void Board::Print() {}
