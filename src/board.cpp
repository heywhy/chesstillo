#include <chesstillo/board.hpp>
#include <chesstillo/types.hpp>
#include <chesstillo/utility.hpp>

bool CoordForSquare(Coord *coord, int square) {
  if (square >= 0 && square < 64) {
    int file = square % 8;
    int rank = square / 8;

    *coord = {static_cast<char>(file + 97), rank + 1};

    return true;
  }

  return false;
}

void Board::Reset() {
  occupied_sqs_ = kEmpty;

  pieces_[WHITE][KING] = pieces_[WHITE][QUEEN] = pieces_[WHITE][ROOK] =
      pieces_[WHITE][KNIGHT] = pieces_[WHITE][BISHOP] = pieces_[WHITE][PAWN] =
          kEmpty;

  pieces_[BLACK][KING] = pieces_[BLACK][QUEEN] = pieces_[BLACK][ROOK] =
      pieces_[BLACK][KNIGHT] = pieces_[BLACK][BISHOP] = pieces_[BLACK][PAWN] =
          kEmpty;
}

bool Board::PieceAtSquare(unsigned int square, char *c) {
  Color color;
  Piece piece;
  bool found = false;
  Bitboard bitboard = BITBOARD_FOR_SQUARE(square);

  for (int i = 0; i < 6; i++) {
    for (int j = 0; j < 2; j++) {
      if (pieces_[j][i] & bitboard) {
        found = true;
        color = static_cast<Color>(j);
        piece = static_cast<Piece>(i);
        break;
      }
    }
  }

  return found && PieceToChar(piece, color, c);
}

bool Board::PieceAtSquare(unsigned int square, Piece *piece) {
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
