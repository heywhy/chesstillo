#ifndef POSITION_HPP
#define POSITION_HPP

#include <cstdint>
#include <stack>
#include <string>
#include <string_view>
#include <utility>

#include "board.hpp"
#include "move.hpp"
#include "types.hpp"

namespace engine {

inline constexpr const char *kStartPos =
    "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

class Position;

struct _State {
  Bitboard king_ban;
  Bitboard occupied_sqs;
  Bitboard en_passant_square;
  Bitboard en_passant_target;
  std::uint_fast8_t castling_rights;
  std::uint_fast8_t halfmove_clock;

  static _State From(Position &position);
  static void Apply(Position &position, _State &state);
};

static const std::stack<_State> kEmptyStack;

// NOTE: maybe compute position hash on every move instead of at TT.
class Position {
 public:
  Position();
  Position(const Position &src);

  static Position FromFen(const std::string_view fen);

  std::string ToFen() const;

  void Reset();
  void Make(const Move &move);
  void Undo(const Move &move);
  bool PieceAt(char *, std::uint_fast8_t) const;
  bool PieceAt(Piece *, std::uint_fast8_t) const;

  Color GetTurn() const { return turn_; }

  inline Bitboard EnPassantSquare() const { return en_passant_sq_; }

  inline bool CanCastle(std::uint_fast8_t direction) const {
    return castling_rights_ & (static_cast<std::uint_fast8_t>(1) << direction);
  }

 private:
  Color turn_;
  Board board_;
  Bitboard king_ban_;
  Bitboard en_passant_sq_;
  Bitboard en_passant_target_;
  std::uint_fast8_t castling_rights_;

  long fullmove_counter_;
  std::uint_fast8_t halfmove_clock_;

  Mailbox mailbox_;
  std::stack<_State> history_;

  void UpdateKingBan();
  void UpdateMailbox();
  void UpdateInternals();

  friend struct _State;
  friend struct EvalState;
  friend class TT;
  friend class SearchManager;

  friend int SEE(Position &position);
  friend int Evaluate(Position &position);
  friend MoveList GenerateMoves(Position &position);
  friend Bitboard CheckMask(Position &position);
  friend std::pair<Bitboard, Bitboard> PinMask(Position &position);

  inline PieceList &Pieces(Color color) { return board_.pieces[color]; }
};

}  // namespace engine

#endif
