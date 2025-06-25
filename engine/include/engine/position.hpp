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

using Castling = std::uint8_t;
using Mailbox = std::array<Piece, 64>;

inline constexpr const char *kStartPos =
    "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

class Position;

namespace position {

inline constexpr Castling CASTLE_W_KING_SIDE = static_cast<Castling>(1) << 0;
inline constexpr Castling CASTLE_W_QUEEN_SIDE = static_cast<Castling>(1) << 1;
inline constexpr Castling CASTLE_B_KING_SIDE = static_cast<Castling>(1) << 2;
inline constexpr Castling CASTLE_B_QUEEN_SIDE = static_cast<Castling>(1) << 3;

struct State {
  Bitboard king_ban;
  Bitboard occupied_sqs;
  Bitboard en_passant_square;
  Bitboard en_passant_target;
  Castling castling_rights;
  std::uint8_t halfmove_clock;

  static State From(Position &position);
  static void Apply(Position &position, State &state);
};

}  // namespace position

static const std::stack<position::State> kEmptyStack;

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
  bool PieceAt(char *, int index) const;
  bool PieceAt(Piece *, int index) const;

  Color GetTurn() const { return turn_; }

  inline Bitboard EnPassantSquare() const { return en_passant_sq_; }

  inline bool CanCastle(Castling flag) const { return castling_rights_ & flag; }

 private:
  Color turn_;
  Board board_;
  Bitboard king_ban_;
  Bitboard en_passant_sq_;
  Bitboard en_passant_target_;
  std::uint8_t castling_rights_;

  long fullmove_counter_;
  std::uint8_t halfmove_clock_;

  Mailbox mailbox_;
  std::stack<position::State> history_;

  void UpdateKingBan();
  void UpdateMailbox();
  void UpdateInternals();

  friend struct position::State;

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
