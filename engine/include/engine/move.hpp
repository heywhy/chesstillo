#ifndef MOVE_HPP
#define MOVE_HPP

#include <cstddef>
#include <cstdint>
#include <forward_list>
#include <vector>

#include "config.hpp"
#include "types.hpp"

namespace engine {
namespace move {

using Flag = std::uint8_t;

constexpr Flag CHECK = static_cast<Flag>(1) << 0;
constexpr Flag CAPTURE = static_cast<Flag>(1) << 1;
constexpr Flag EN_PASSANT = static_cast<Flag>(1) << 2;
constexpr Flag CASTLE_KING_SIDE = static_cast<Flag>(1) << 3;
constexpr Flag CASTLE_QUEEN_SIDE = static_cast<Flag>(1) << 4;
constexpr Flag CHECKMATE = static_cast<Flag>(1) << 5;
constexpr Flag PROMOTION = static_cast<Flag>(1) << 6;
// constexpr Flag DISCOVERY = static_cast<Flag>(1) << 0;
// constexpr Flag DOUBLE = static_cast<Flag>(1) << 0;

}  // namespace move

class Move;
using MoveList = std::vector<Move>;

class Move {
 public:
  std::uint8_t from;
  std::uint8_t to;
  move::Flag flags;

  Piece piece;
  Piece captured;
  Piece promoted;

  int cost = 0;
  int score;

  Move() : Move(0, 0, Piece::NONE) {}

  Move(std::uint8_t from, std::uint8_t to, Piece piece)
      : Move(from, to, piece, 0) {}

  Move(std::uint8_t from, std::uint8_t to, Piece piece, std::uint8_t flags)
      : from(from), to(to), flags(flags), piece(piece), score(MIN_SCORE) {}

  void Set(move::Flag flag) { flags |= flag; }

  bool Is(move::Flag flag) const { return flags & flag; }

  friend inline bool operator==(const Move &lhs, const Move &rhs) {
    return lhs.from == rhs.from && lhs.to == rhs.to && lhs.piece == rhs.piece;
  }
};

struct Line {
  Color color;
  std::forward_list<Move> moves;
};

}  // namespace engine

#endif
