#include <cstdint>
#include <list>
#include <string>

#include "types.hpp"

#ifndef BOARD_HPP

#define BOARD_HPP

#define BITBOARD_FOR_SQUARE(index) static_cast<Bitboard>(1) << index

const Bitboard kEmpty = 0;
const Bitboard kUniverse = -1;

enum Castling { K_WHITE, Q_WHITE, K_BLACK, Q_BLACK };

struct Coord {
  char file;
  int rank;
};

Bitboard BitboardForSquare(int file, int rank);
Bitboard BitboardForSquare(char file, int rank);
bool CoordFromBitboard(Bitboard square, Coord *coord);
int SquareFromBitboard(Bitboard bb);

/**
 * TODO:
 *
 * castling
 * en passant
 */
class Board {
public:
  void Reset();
  bool Endgame();
  void ApplyMove(Move move);
  void Print();
  bool PieceAtSquare(Bitboard square, char *);
  bool PieceAtSquare(Bitboard square, Piece *);
  Bitboard EmptySquares() { return ~occupied_sqs_; }

  Bitboard Position(Piece piece, Color color) {
    return color == WHITE ? w_pieces_[piece] : b_pieces_[piece];
  }

  bool CanCastle(Castling direction) {
    return castling_rights_ & (static_cast<std::uint8_t>(1) << direction);
  }

  Color GetTurn() { return turn_; }
  std::list<Move> GetMoves() { return moves_; }
  Bitboard EnPassantSquare() { return en_passant_sq_; }

private:
  int halfmove_clock_;
  Color turn_ = WHITE;
  std::list<Move> moves_;
  long fullmove_counter_ = 1;
  std::uint8_t castling_rights_;

  // pawn, rook, knight, bishop, queen, king
  Bitboard w_pieces_[6];
  Bitboard b_pieces_[6];
  Bitboard w_attacking_sqs_[6];
  Bitboard b_attacking_sqs_[6];

  Bitboard occupied_sqs_ = kEmpty;
  Bitboard en_passant_sq_ = kEmpty;
  Bitboard sqs_occupied_by_w_;
  Bitboard sqs_occupied_by_b_;

  friend void ApplyFen(Board &board, const char *fen);
  friend std::string PositionToFen(Board &board);
  friend bool IsValidPawnMove(Board &board, Bitboard const piece,
                              Move const &move, Bitboard const &attacking_sqs);
  friend bool IsValidSlidingMove(Board &board, Bitboard const bb,
                                 Move const &move);

  void ComputeAttackedSqs();
  bool IsValidMove(Move const &move);

  inline Bitboard SquaresOccupiedByOpp(Color color) {
    return color == WHITE ? sqs_occupied_by_b_ : sqs_occupied_by_w_;
  }

  void ComputeOccupiedSqs() {
    sqs_occupied_by_w_ = kEmpty;
    sqs_occupied_by_b_ = kEmpty;

    for (int i = 0; i < 6; i++) {
      sqs_occupied_by_w_ |= w_pieces_[i];
      sqs_occupied_by_b_ |= b_pieces_[i];
    }

    occupied_sqs_ = sqs_occupied_by_w_ | sqs_occupied_by_b_;
  }
};

#endif
