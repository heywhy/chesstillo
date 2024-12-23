#ifndef POSITION_HPP
#define POSITION_HPP

#include <algorithm>
#include <cstdint>
#include <stack>
#include <string>
#include <utility>
#include <vector>

#include "board.hpp"
#include "constants.hpp"
#include "types.hpp"

class Position;

struct _State {
  Bitboard king_ban;
  Bitboard occupied_sqs;
  Bitboard en_passant_square;
  Bitboard en_passant_target;
  uint8_t castling_rights;
  uint8_t halfmove_clock;

  static _State From(Position &position);
  static void Apply(Position &position, _State &state);
};

static std::stack<_State> const kEmptyStack;

// NOTE: maybe compute position hash on every move instead of at TT.
class Position {
public:
  Position()
      : turn_(WHITE), king_ban_(kEmpty), occupied_sqs_(&board_.occupied_sqs_),
        en_passant_sq_(kEmpty), castling_rights_(0), fullmove_counter_(1),
        halfmove_clock_(0) {};

  Position(const Position &src) {
    turn_ = src.turn_;
    king_ban_ = src.king_ban_;
    en_passant_sq_ = src.en_passant_sq_;
    en_passant_target_ = src.en_passant_target_;
    castling_rights_ = src.castling_rights_;
    fullmove_counter_ = src.fullmove_counter_;
    halfmove_clock_ = src.halfmove_clock_;

    history_ = src.history_;
    board_ = src.board_;

    occupied_sqs_ = &board_.occupied_sqs_;

    int size = sizeof(src.mailbox_) / sizeof(src.mailbox_[0]);
    std::copy(src.mailbox_, src.mailbox_ + size, mailbox_);
  }

  void Reset();
  void Make(Move move);
  void Undo(Move &move);
  bool PieceAt(char *, uint8_t);
  bool PieceAt(Piece *, uint8_t);

  Color GetTurn() { return turn_; }
  Bitboard OccupiedSquares() { return *occupied_sqs_; }
  inline Bitboard EnPassantSquare() { return en_passant_sq_; }

  inline bool CanCastle(uint8_t direction) {
    return castling_rights_ & (static_cast<std::uint8_t>(1) << direction);
  }

private:
  Color turn_;
  Board board_;
  Bitboard king_ban_;
  Bitboard *occupied_sqs_;
  Bitboard en_passant_sq_;
  Bitboard en_passant_target_;
  uint8_t castling_rights_;

  long fullmove_counter_;
  uint8_t halfmove_clock_;

  std::stack<_State> history_;
  Piece mailbox_[64];

  void UpdateKingBan();
  void UpdateMailbox();
  void UpdateInternals();

  friend struct _State;
  friend struct EvalState;
  friend class TT;

  friend int SEE(Position &position);
  friend int Evaluate(Position &position);
  friend std::vector<Move> GenerateMoves(Position &position);
  friend Bitboard CheckMask(Position &position);
  friend std::pair<Bitboard, Bitboard> PinMask(Position &position);
  friend std::string PositionToFen(Position &position);
  friend void ApplyFen(Position &position, const char *fen);

  inline Bitboard *Pieces(Color color) { return board_.pieces_[color]; }
};

#endif
