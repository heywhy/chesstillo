#ifndef POSITION_HPP
#define POSITION_HPP

#include <list>

#include "board.hpp"
#include "constants.hpp"
#include "types.hpp"

class Position {
public:
  Position()
      : turn_(WHITE), king_ban_(kEmpty), en_passant_sq_(kEmpty),
        castling_rights_(0), fullmove_counter_(1), halfmove_clock_(0){};

  void Reset();
  void Make(Move move);
  void Undo(Move &move);

  Color GetTurn() { return turn_; }
  std::list<Move> &GetMoves() { return moves_; }
  Bitboard EnPassantSquare() { return en_passant_sq_; }
  Bitboard OccupiedSquares() { return board_.occupied_sqs_; }

  bool CanCastle(Castling direction) {
    return castling_rights_ & (static_cast<std::uint8_t>(1) << direction);
  }

private:
  Color turn_;
  Board board_;
  Bitboard king_ban_;
  Bitboard en_passant_sq_;
  std::uint8_t castling_rights_;

  std::list<Move> moves_;
  long fullmove_counter_;
  unsigned int halfmove_clock_;

  void UpdateKingBan();
  void UpdateInternals();
  void UpdateEnPassantSquare(Move &last_move);

  Bitboard EnPassantTarget();

  friend std::vector<Move> GenerateMoves(Position &position);
  friend Bitboard CheckMask(Position &position);
  friend std::tuple<Bitboard, Bitboard> PinMask(Position &position);
  friend std::string PositionToFen(Position &position);
  friend void ApplyFen(Position &position, const char *fen);
  friend Move DeduceMove(Position &position, unsigned int from,
                         unsigned int to);

  bool PieceAtSquare(unsigned int square, char *piece) {
    return board_.PieceAtSquare(square, piece);
  }

  bool PieceAtSquare(unsigned int square, Piece *piece) {
    return board_.PieceAtSquare(square, piece);
  }

  inline Bitboard *Pieces(Color color) { return board_.pieces_[color]; }
};

#endif
