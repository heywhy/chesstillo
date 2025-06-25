#ifndef ENGINE_SQUARE_HPP
#define ENGINE_SQUARE_HPP

#include <bit>

#include "types.hpp"

namespace engine {
namespace square {

constexpr Bitboard BB(const int square) {
  return static_cast<Bitboard>(1) << square;
}

constexpr int Index(const Bitboard bb) { return std::countr_zero(bb); }

constexpr int MIndex(const Bitboard bb) { return 63 - std::countl_zero(bb); }

constexpr Bitboard Occupancy(const Bitboard* pieces) {
  return pieces[KING] | pieces[QUEEN] | pieces[BISHOP] | pieces[KNIGHT] |
         pieces[ROOK] | pieces[PAWN];
}

constexpr Bitboard Occupancy(const PieceList& pieces) {
  return Occupancy(pieces.data());
}

constexpr int Rank(const int square) { return (square / 8) + 1; }

constexpr int File(const int square) { return square % 8; }

constexpr int From(const int file, const int rank) { return 8 * rank + file; }

}  // namespace square
}  // namespace engine

#endif
