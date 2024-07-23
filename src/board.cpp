#include <chesstillo/board.hpp>
#include <utility>

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

  return static_cast<Bitboard>(1) << square;
}

int SquareFromBitboard(Bitboard bb) {
  Coord coord;

  if (CoordFromBitboard(bb, &coord)) {
    return 8 * (coord.rank - 1) + ToFile(coord.file);
  }

  return -1;
}

bool CoordFromBitboard(Bitboard square, Coord *coord) {
  if (square) {
    int index = std::popcount((square & -square) - 1);
    int file = index % 8;
    int rank = index / 8;

    *coord = {kFiles[file], rank + 1};

    return true;
  }

  return false;
}

Bitboard BitboardForSquare(char file, int rank) {
  return BitboardForSquare(ToFile(file), rank - 1);
}

void Board::Reset() {
  for (int i = 0; i < 6; i++) {
    b_pieces_[i] = b_attacking_sqs_[i] = kEmpty;
    w_pieces_[i] = w_attacking_sqs_[i] = kEmpty;
  }

  turn_ = WHITE;
  castling_rights_ = 0;
  en_passant_sq_ = kEmpty;

  occupied_sqs_ = kEmpty;
  sqs_occupied_by_b_ = kEmpty;
  sqs_occupied_by_w_ = kEmpty;
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

  moves_.push_front(std::move(move));
}

bool Board::IsValidMove(Move const &move) {
  Bitboard piece =
      move.color == WHITE ? w_pieces_[move.piece] : b_pieces_[move.piece];

  Bitboard occupied_sqs =
      move.color == WHITE ? sqs_occupied_by_w_ : sqs_occupied_by_b_;

  Bitboard attacking_sqs = move.color == WHITE ? w_attacking_sqs_[move.piece]
                                               : b_attacking_sqs_[move.piece];

  piece &= move.from;

  if (!piece)
    return false;

  switch (move.piece) {
  case PAWN:
    return IsValidPawnMove(*this, piece, move, attacking_sqs);

  case BISHOP:
  case QUEEN:
  case ROOK:
    return IsValidSlidingMove(*this, piece, move);

  case KING:
  case KNIGHT:
    // use the same logic used by sliding pieces to filter own square
    return (attacking_sqs ^ occupied_sqs) & attacking_sqs & move.to;

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

  w_attacking_sqs_[BISHOP] = b_attacking_sqs_[BISHOP] = kEmpty;
  w_attacking_sqs_[ROOK] = b_attacking_sqs_[ROOK] = kEmpty;
  w_attacking_sqs_[QUEEN] = b_attacking_sqs_[QUEEN] = kEmpty;
}

char Board::PieceAtSquare(Bitboard square) {
  Color color;
  Piece piece;
  char c = '\0';

  for (int i = 0; i < 6; i++) {
    if (w_pieces_[i] & square) {
      color = WHITE;
      piece = static_cast<Piece>(i);
      break;
    }

    if (b_pieces_[i] & square) {
      color = BLACK;
      piece = static_cast<Piece>(i);
      break;
    }
  }

  switch (piece) {
  case ROOK:
    c = 'r';
    break;
  case KNIGHT:
    c = 'n';
    break;
  case BISHOP:
    c = 'b';
    break;
  case QUEEN:
    c = 'q';
    break;
  case KING:
    c = 'k';
    break;
  case PAWN:
    c = 'p';
    break;
  default:
    return c;
  }

  return color == BLACK ? c : toupper(c);
}
