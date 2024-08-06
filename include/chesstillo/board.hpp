#include <cstdint>
#include <list>
#include <string>
#include <vector>

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
  void UndoMove(Move &move);
  void Print();
  bool PieceAtSquare(Bitboard square, char *);
  bool PieceAtSquare(Bitboard square, Piece *);
  Bitboard EmptySquares() { return ~occupied_sqs_; }

  Bitboard Position(Piece piece, Color color) { return pieces_[color][piece]; }

  bool CanCastle(Castling direction) {
    return castling_rights_ & (static_cast<std::uint8_t>(1) << direction);
  }

  Color GetTurn() { return turn_; }
  std::list<Move> &GetMoves() { return moves_; }
  Bitboard EnPassantSquare() { return en_passant_sq_; }

private:
  int halfmove_clock_;
  Color turn_ = WHITE;
  std::list<Move> moves_;
  long fullmove_counter_ = 1;
  std::uint8_t castling_rights_;

  // pawn, rook, knight, bishop, queen, king
  Bitboard pieces_[2][6];
  Bitboard attacking_sqs_[2][6];
  Bitboard sqs_attacked_by_[2];
  Bitboard sqs_occupied_by_[2];

  Bitboard occupied_sqs_ = kEmpty;
  Bitboard en_passant_sq_ = kEmpty;

  friend void ApplyFen(Board &board, const char *fen);
  friend std::string PositionToFen(Board &board);
  friend std::vector<Move> GenerateMoves(Board &board);
  friend std::vector<Move> GenerateOutOfCheckMoves(Board &board);
  friend std::vector<Move> GenerateMoves(Board &board, Piece piece);
  friend std::size_t GenerateMoves(Board &board, Piece piece, Bitboard square,
                                   Bitboard *const out);

  friend std::size_t GeneratePawnMoves(Board &board, Bitboard square,
                                       Bitboard *const out);
  friend std::size_t GenerateKnightMoves(Board &board, Bitboard square,
                                         Bitboard *const out);
  friend std::size_t GenerateSlidingPieceMoves(Board &board, Piece piece,
                                               Bitboard square,
                                               Bitboard *const out);

  friend std::size_t GenerateKingMoves(Board &board, Bitboard square,
                                       Bitboard *const out);

  void ComputeAttackedSqs();
  void ComputeEnPassantSquare();
  Bitboard EnPassantTarget();
  void MakeMove(Move &move);
  bool IsValidMove(Move const &move);

  void ComputeOccupiedSqs() {
    sqs_occupied_by_[WHITE] = kEmpty;
    sqs_occupied_by_[BLACK] = kEmpty;

    for (int i = 0; i < 6; i++) {
      sqs_occupied_by_[WHITE] |= pieces_[WHITE][i];
      sqs_occupied_by_[BLACK] |= pieces_[BLACK][i];
    }

    occupied_sqs_ = sqs_occupied_by_[WHITE] | sqs_occupied_by_[BLACK];
  }
};

#endif
