#include <cstdint>
#include <list>
#include <string>

#include "move.hpp"
#include "types.hpp"

#ifndef BOARD_HPP

#define BOARD_HPP

const Bitboard EMPTY = 0;
const Bitboard UNIVERSE = -1;

enum File { a, b, c, d, e, f, g, h };
enum Castling { K_WHITE, Q_WHITE, K_BLACK, Q_BLACK };

Bitboard BitboardForSquare(File file, int rank);

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
  Bitboard EmptySquares() { return ~occupied_sqs_; }

  Bitboard Position(Piece piece, Color color) {
    return color == WHITE ? w_pieces_[piece] : b_pieces_[piece];
  }

  bool CanCastle(Castling direction) {
    return castling_rights_ & (1 << direction);
  }

private:
  Color turn_;
  std::uint8_t castling_rights_;

  // pawn, rook, knight, bishop, queen, king
  Bitboard w_pieces_[6];
  Bitboard b_pieces_[6];

  Bitboard occupied_sqs_ = EMPTY;
  Bitboard sqs_occupied_by_w_;
  Bitboard sqs_occupied_by_b_;

  std::list<Move> moves;

  friend Board &ApplyFen(Board &board, const char *fen);
  friend std::string PositionToFen(Board &board);

  void ComputeOccupied() {
    sqs_occupied_by_w_ = EMPTY;
    sqs_occupied_by_b_ = EMPTY;

    for (int i = 0; i < 6; i++) {
      sqs_occupied_by_w_ |= w_pieces_[i];
    }

    for (int i = 0; i < 6; i++) {
      sqs_occupied_by_b_ |= b_pieces_[i];
    }

    occupied_sqs_ = sqs_occupied_by_w_ | sqs_occupied_by_b_;
  }

  char PieceAtSquare(Bitboard square) {
    char c;
    Color color;
    Piece piece;

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
    }

    return color == BLACK ? c : toupper(c);
  }
};

#endif
