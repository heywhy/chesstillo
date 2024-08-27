#ifndef BOARD_HPP
#define BOARD_HPP

#include "constants.hpp"
#include "types.hpp"

#define OPP(turn) static_cast<Color>(turn ^ 1)
#define TO_SQUARE(file, rank) 8 * rank + file
#define BIT_INDEX(x) std::countr_zero(x)
#define BIT_INDEX_MSB(x) (63 - std::countl_zero(x))
#define BITBOARD_FOR_SQUARE(index) static_cast<Bitboard>(1) << index
#define BOARD_OCCUPANCY(pieces)                                                \
  (pieces[KING] | pieces[QUEEN] | pieces[BISHOP] | pieces[KNIGHT] |            \
   pieces[ROOK] | pieces[PAWN])

struct Coord {
  char file;
  int rank;
};

bool CoordForSquare(Coord *coord, int square);

/**
 * TODO:
 *
 * castling
 */
class Board {
public:
  Board() : occupied_sqs_(kEmpty) {}

  void Reset();
  bool PieceAtSquare(unsigned int square, char *);
  bool PieceAtSquare(unsigned int square, Piece *);
  Bitboard EmptySquares() { return ~occupied_sqs_; }
  Bitboard PiecesAttackingSquare(unsigned int square, Color color);

  Bitboard Position(Piece piece, Color color) { return pieces_[color][piece]; }

private:
  // pawn, rook, knight, bishop, queen, king
  Bitboard pieces_[2][6];
  Bitboard occupied_sqs_;

  friend class Position;

  void UpdateOccupiedSqs() {
    occupied_sqs_ =
        BOARD_OCCUPANCY(pieces_[WHITE]) | BOARD_OCCUPANCY(pieces_[BLACK]);
  }
};

#endif
