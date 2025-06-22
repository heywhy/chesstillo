#ifndef MOVE_HPP
#define MOVE_HPP

#include <cstddef>
#include <cstdint>
#include <forward_list>
#include <vector>

#include "types.hpp"

namespace engine {

class Move;
typedef std::vector<Move> MoveList;

class Move {
 public:
  std::uint_fast8_t from;
  std::uint_fast8_t to;
  Piece piece;
  std::uint_fast8_t flags;
  Piece captured;
  Piece promoted;

  int cost = 0;
  int score;

  Move *next;

  static const Move NONE;

  Move(std::uint_fast8_t from, std::uint_fast8_t to, Piece piece)
      : from(from), to(to), piece(piece), flags(0), score(SCORE_INF) {}

  Move(std::uint_fast8_t from, std::uint_fast8_t to, Piece piece,
       std::uint_fast8_t flags)
      : from(from), to(to), piece(piece), flags(flags), score(SCORE_INF) {}

  void Set(std::uint_fast8_t flag) { flags |= 1U << flag; }

  bool Is(std::uint_fast8_t flag) const { return flags & (1U << flag); }

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
