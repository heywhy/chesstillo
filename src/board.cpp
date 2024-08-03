#include <algorithm>
#include <utility>
#include <vector>

#include <chesstillo/board.hpp>
#include <chesstillo/move.hpp>
#include <chesstillo/types.hpp>

#include "fill.hpp"

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

  return BITBOARD_FOR_SQUARE(square);
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
    pieces_[WHITE][i] = attacking_sqs_[WHITE][i] = kEmpty;
    pieces_[BLACK][i] = attacking_sqs_[BLACK][i] = kEmpty;
  }

  turn_ = WHITE;
  castling_rights_ = 0;
  en_passant_sq_ = kEmpty;

  occupied_sqs_ = kEmpty;
  sqs_occupied_by_[WHITE] = kEmpty;
  sqs_occupied_by_[BLACK] = kEmpty;

  moves_.clear();
}

bool Board::Endgame() { return false; }

void Board::ApplyMove(Move move) {
  if (turn_ != move.color || !IsValidMove(move))
    return;

  MakeMove(move);
}

void Board::MakeMove(Move &move) {
  Color opp = static_cast<Color>(move.color ^ 1);
  Bitboard &piece = pieces_[move.color][move.piece];

  piece ^= move.from ^ move.to;

  if (move.to & sqs_occupied_by_[opp]) {
    Bitboard *pieces = pieces_[opp];

    for (int i = 0; i < 6; i++) {
      if (pieces[i] & move.to) {
        move.Set(CAPTURE);

        move.captured = static_cast<Piece>(i);
        pieces[i] ^= move.to;
        break;
      }
    }
  }

  ComputeOccupiedSqs();
  ComputeAttackedSqs();

  if (sqs_attacked_by_[turn_] & pieces_[opp][KING]) {
    // INFO: Copying the board just to see if the
    turn_ = opp;

    move.Set(CHECK);

    if (GenerateOutOfCheckMoves(*this).empty()) {
      move.Set(CHECKMATE);
    }

    turn_ = move.color;
  }

  if (sqs_occupied_by_[opp] && !move.Is(CHECKMATE)) {
    turn_ = opp;
  }

  if (move.piece == PAWN || move.Is(CAPTURE)) {
    halfmove_clock_ = 0;
  } else {
    halfmove_clock_++;
  }

  if (move.color == BLACK)
    fullmove_counter_++;

  moves_.push_front(std::move(move));
}

void Board::UndoMove(Move &move) {
  if (move != moves_.front()) {
    return;
  }

  Bitboard &piece = pieces_[move.color][move.piece];

  piece = (piece ^ move.to) | move.from;
  turn_ = move.color;

  if (move.Is(CAPTURE)) {
    Bitboard *pieces = pieces_[move.color ^ 1];

    pieces[move.captured] |= move.to;
  }

  if (move.Is(CAPTURE) || move.piece == PAWN) {
    halfmove_clock_ = 0;
  }

  if (move.color == BLACK) {
    fullmove_counter_--;
  }

  ComputeOccupiedSqs();
  ComputeAttackedSqs();

  moves_.pop_front();
}

bool Board::IsValidMove(Move const &move) {
  if (!moves_.empty() && moves_.front().Is(CHECKMATE)) {
    return false;
  }

  std::vector<Move> moves = GenerateMoves(*this, move.piece);

  auto it = std::find_if(moves.begin(), moves.end(), [&](Move &generated) {
    return generated.from == move.from & move.to == generated.to &&
           move.piece == generated.piece;
  });

  return it != std::end(moves);
}

void Board::ComputeAttackedSqs() {
  for (int color = 0; color < 2; color++) {
    attacking_sqs_[color][KNIGHT] = KNIGHT_ATTACKS(pieces_[color][KNIGHT]);

    attacking_sqs_[color][KNIGHT] &=
        attacking_sqs_[color][KNIGHT] ^ sqs_occupied_by_[color];

    attacking_sqs_[color][KING] = KING_ATTACKS(pieces_[color][KING]);

    attacking_sqs_[color][KING] &=
        attacking_sqs_[color][KING] ^ sqs_occupied_by_[color];

    attacking_sqs_[color][BISHOP] =
        BISHOP_ATTACKS(pieces_[color][BISHOP], ~occupied_sqs_);

    attacking_sqs_[color][BISHOP] &=
        attacking_sqs_[color][BISHOP] ^ sqs_occupied_by_[color];

    attacking_sqs_[color][ROOK] =
        ROOK_ATTACKS(pieces_[color][ROOK], ~occupied_sqs_);

    attacking_sqs_[color][ROOK] &=
        attacking_sqs_[color][ROOK] ^ sqs_occupied_by_[color];

    attacking_sqs_[color][QUEEN] =
        (ROOK_ATTACKS(pieces_[color][QUEEN], ~occupied_sqs_) |
         BISHOP_ATTACKS(pieces_[color][QUEEN], ~occupied_sqs_));

    attacking_sqs_[color][QUEEN] &=
        attacking_sqs_[color][QUEEN] ^ sqs_occupied_by_[color];
  }

  attacking_sqs_[WHITE][PAWN] = (MOVE_NORTH_EAST(pieces_[WHITE][PAWN]) ^
                                 MOVE_NORTH_WEST(pieces_[WHITE][PAWN])) |
                                (MOVE_NORTH_EAST(pieces_[WHITE][PAWN]) &
                                 MOVE_NORTH_WEST(pieces_[WHITE][PAWN]));

  attacking_sqs_[WHITE][PAWN] &=
      attacking_sqs_[WHITE][PAWN] ^ sqs_occupied_by_[WHITE];

  attacking_sqs_[BLACK][PAWN] = (MOVE_SOUTH_EAST(pieces_[BLACK][PAWN]) ^
                                 MOVE_SOUTH_WEST(pieces_[BLACK][PAWN])) |
                                (MOVE_SOUTH_EAST(pieces_[BLACK][PAWN]) &
                                 MOVE_SOUTH_WEST(pieces_[BLACK][PAWN]));

  attacking_sqs_[BLACK][PAWN] &=
      attacking_sqs_[BLACK][PAWN] ^ sqs_occupied_by_[BLACK];

  sqs_attacked_by_[WHITE] = kEmpty;
  sqs_attacked_by_[BLACK] = kEmpty;

  for (int piece = 0; piece < 6; piece++) {
    sqs_attacked_by_[WHITE] |= attacking_sqs_[WHITE][piece];
    sqs_attacked_by_[BLACK] |= attacking_sqs_[BLACK][piece];
  }
}

bool Board::PieceAtSquare(Bitboard square, char *c) {
  Color color;
  Piece piece;

  for (int i = 0; i < 6; i++) {
    for (int j = 0; j < 2; j++) {
      if (pieces_[j][i] & square) {
        color = static_cast<Color>(j);
        piece = static_cast<Piece>(i);
        break;
      }
    }
  }

  return PieceToChar(piece, color, c);
}

bool Board::PieceAtSquare(Bitboard square, Piece *piece) {
  char c;

  if (!PieceAtSquare(square, &c)) {
    return false;
  }

  switch (c) {
  case 'r':
  case 'R':
    *piece = ROOK;
    break;

  case 'n':
  case 'N':
    *piece = KNIGHT;
    break;

  case 'b':
  case 'B':
    *piece = BISHOP;
    break;

  case 'k':
  case 'K':
    *piece = KING;
    break;

  case 'q':
  case 'Q':
    *piece = QUEEN;
    break;

  case 'p':
  case 'P':
    *piece = PAWN;
    break;
  }

  return true;
}
