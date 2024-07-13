#include "board.hpp"
#include "move.hpp"

static const char kFiles[] = {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h'};
static const int kFilesLength = sizeof(kFiles) / sizeof(char);

int ToFile(char file) {
  const char *element = std::find(&kFiles[0], kFiles + kFilesLength, file);

  return element - kFiles;
}

Bitboard BitboardForSquare(int file, int rank) {
  // int square = 8 * file + rank;
  // int square = 8 * (rank - 1) + file;
  int square = 8 * rank + file;

  return 1ULL << square;
}

int SquareFromBitboard(Bitboard bb) {
  Coord coord = CoordFromBitboard(bb);

  return coord.IsValid() ? 8 * (coord.rank - 1) + ToFile(coord.file) : -1;
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
  return BitboardForSquare(ToFile(file), rank - 1);
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

  Bitboard *piece =
      move.color == WHITE ? &w_pieces_[move.piece] : &b_pieces_[move.piece];

  *piece ^= move.from ^ move.to;

  if (SquaresOccupiedByOpp(move.color) & move.to) {
    Bitboard *pieces = move.color == WHITE ? b_pieces_ : w_pieces_;

    SetFlags(&move, CAPTURE);

    for (int i = 0; i < 6; i++) {
      if (pieces[i] & move.to) {
        pieces[i] ^= move.to;
        break;
      }
    }
  }

  moves.push_front(move);

  if (move.piece == PAWN || move.IsCapture()) {
    halfmove_clock_ = 0;
  } else {
    halfmove_clock_++;
  }

  if (move.color == BLACK)
    fullmove_counter_++;

  ComputeOccupiedSqs();
  ComputeAttackedSqs();

  if (SquaresOccupiedByOpp(move.color)) {
    turn_ = move.color == WHITE ? BLACK : WHITE;
  }
}

bool Board::IsValidMove(Move const &move) {
  Bitboard piece =
      move.color == WHITE ? w_pieces_[move.piece] : b_pieces_[move.piece];

  Bitboard attacking_sqs = move.color == WHITE ? w_attacking_sqs_[move.piece]
                                               : b_attacking_sqs_[move.piece];

  piece &= move.from;

  if (!piece)
    return false;

  switch (move.piece) {
  case PAWN:
    return IsValidPawnMove(*this, piece, move, attacking_sqs);

  case KNIGHT:
    return IsValidKnightMove(*this, piece, move, attacking_sqs);

  case BISHOP:
  case QUEEN:
  case ROOK:
    return IsValidSlidingMove(*this, piece, move);

  case KING:
    return attacking_sqs & move.to;

  default:
    return false;
  }
}

void Board::ComputeAttackedSqs() {
  w_attacking_sqs_[PAWN] =
      (MOVE_NORTH_EAST(w_pieces_[PAWN]) ^ MOVE_NORTH_WEST(w_pieces_[PAWN])) |
      (MOVE_NORTH_EAST(w_pieces_[PAWN]) & MOVE_NORTH_WEST(w_pieces_[PAWN]));

  b_attacking_sqs_[PAWN] =
      (MOVE_SOUTH_EAST(b_pieces_[PAWN]) ^ MOVE_SOUTH_WEST(b_pieces_[PAWN])) |
      (MOVE_SOUTH_EAST(b_pieces_[PAWN]) & MOVE_SOUTH_WEST(b_pieces_[PAWN]));

  w_attacking_sqs_[KNIGHT] = KNIGHT_ATTACKS(w_pieces_[KNIGHT]);
  b_attacking_sqs_[KNIGHT] = KNIGHT_ATTACKS(b_pieces_[KNIGHT]);

  w_attacking_sqs_[KING] = KING_ATTACKS(w_pieces_[KING]);
  b_attacking_sqs_[KING] = KING_ATTACKS(b_pieces_[KING]);
}

void Board::Print() {}
