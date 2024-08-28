#ifndef POSITION_HPP
#define POSITION_HPP

#include <cstdint>
#include <list>
#include <stack>

#include "board.hpp"
#include "constants.hpp"
#include "types.hpp"

class Position;

struct _State {
  Bitboard king_ban;
  Bitboard occupied_sqs;
  Bitboard en_passant_square;
  Bitboard en_passant_target;
  std::uint8_t castling_rights;
  unsigned int halfmove_clock;

  static _State From(Position &position);
  static void Apply(Position &position, _State &state);
};

static std::stack<_State> const kEmptyStack;

class Position {
public:
  Position()
      : turn_(WHITE), king_ban_(kEmpty), en_passant_sq_(kEmpty),
        castling_rights_(0), fullmove_counter_(1), halfmove_clock_(0) {
    occupied_sqs_ = &board_.occupied_sqs_;
  };

  void Reset();
  void Make(Move move);
  void Undo(Move &move);

  Color GetTurn() { return turn_; }
  std::list<Move> &GetMoves() { return moves_; }
  Bitboard EnPassantSquare() { return en_passant_sq_; }
  Bitboard OccupiedSquares() { return *occupied_sqs_; }

  inline bool CanCastle(std::uint8_t direction) {
    return castling_rights_ & (static_cast<std::uint8_t>(1) << direction);
  }

private:
  Color turn_;
  Board board_;
  Bitboard king_ban_;
  Bitboard *occupied_sqs_;
  Bitboard en_passant_sq_;
  Bitboard en_passant_target_;
  std::uint8_t castling_rights_;

  std::list<Move> moves_;
  long fullmove_counter_;
  unsigned int halfmove_clock_;

  std::stack<_State> history_;

  void UpdateKingBan();
  void UpdateInternals();
  inline void UpdateEnPassantSquare(Move &last_move);

  friend struct _State;

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
