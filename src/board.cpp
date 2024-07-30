#include <algorithm>
#include <utility>
#include <vector>

#include <chesstillo/board.hpp>
#include <chesstillo/move.hpp>
#include <chesstillo/types.hpp>

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

  Color opp = move.color == WHITE ? BLACK : WHITE;
  Bitboard &piece = pieces_[move.color][move.piece];

  piece ^= move.from ^ move.to;

  if (sqs_occupied_by_[opp] & move.to) {
    Bitboard *pieces = pieces_[opp];

    SetFlags(&move, CAPTURE);

    for (int i = 0; i < 6; i++) {
      if (pieces[i] & move.to) {
        move.captured = static_cast<Piece>(i);
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

  if (sqs_occupied_by_[opp]) {
    turn_ = opp;
  }

  moves_.push_front(std::move(move));
}

void Board::UndoMove(Move _) {
  Move move = moves_.front();

  Bitboard &piece = pieces_[move.color][move.piece];

  piece = (piece ^ move.to) | move.from;
  turn_ = move.color;

  if (move.IsCapture()) {
    Bitboard *pieces = pieces_[move.color == WHITE ? BLACK : WHITE];

    pieces[move.captured] |= move.to;
  }

  if (!moves_.empty()) {
    moves_.pop_front();
  }

  ComputeOccupiedSqs();
  ComputeAttackedSqs();
}

bool Board::IsValidMove(Move const &move) {
  std::vector<Move> moves = GenerateMoves(*this, move.piece);

  auto it = std::find_if(moves.begin(), moves.end(), [&](Move &generated) {
    return generated.from == move.from & move.to == generated.to;
  });

  return it != std::end(moves);
}

void Board::ComputeAttackedSqs() {
  for (int color = 0; color < 2; color++) {
    attacking_sqs_[color][KNIGHT] = KNIGHT_ATTACKS(pieces_[color][KNIGHT]);

    attacking_sqs_[color][KING] = KING_ATTACKS(pieces_[color][KING]);

    attacking_sqs_[color][BISHOP] = kEmpty;
    attacking_sqs_[color][ROOK] = kEmpty;
    attacking_sqs_[color][QUEEN] = kEmpty;
  }

  attacking_sqs_[WHITE][PAWN] = (MOVE_NORTH_EAST(pieces_[WHITE][PAWN]) ^
                                 MOVE_NORTH_WEST(pieces_[WHITE][PAWN])) |
                                (MOVE_NORTH_EAST(pieces_[WHITE][PAWN]) &
                                 MOVE_NORTH_WEST(pieces_[WHITE][PAWN]));

  attacking_sqs_[BLACK][PAWN] = (MOVE_SOUTH_EAST(pieces_[BLACK][PAWN]) ^
                                 MOVE_SOUTH_WEST(pieces_[BLACK][PAWN])) |
                                (MOVE_SOUTH_EAST(pieces_[BLACK][PAWN]) &
                                 MOVE_SOUTH_WEST(pieces_[BLACK][PAWN]));
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
