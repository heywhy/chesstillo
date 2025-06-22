#ifndef ENGINE_SQUARE_HPP
#define ENGINE_SQUARE_HPP

#include <bit>
#include <cstdint>

#include "types.hpp"

namespace engine {
namespace square {

constexpr Bitboard BB(const std::uint_fast8_t square) {
  return Bitboard(1) << square;
}

constexpr std::uint_fast8_t Index(const Bitboard bb) {
  return std::countr_zero(bb);
}

constexpr std::uint_fast8_t MIndex(const Bitboard bb) {
  return 63 - std::countl_zero(bb);
}

constexpr Bitboard Occupancy(const Bitboard* pieces) {
  return pieces[KING] | pieces[QUEEN] | pieces[BISHOP] | pieces[KNIGHT] |
         pieces[ROOK] | pieces[PAWN];
}

constexpr Bitboard Occupancy(const PieceList& pieces) {
  return Occupancy(pieces.data());
  // return pieces[KING] | pieces[QUEEN] | pieces[BISHOP] | pieces[KNIGHT] |
  //        pieces[ROOK] | pieces[PAWN];
}

constexpr std::uint_fast8_t Rank(const std::uint_fast8_t square) {
  return (square / 8) + 1;
}

constexpr std::uint_fast8_t File(const std::uint_fast8_t square) {
  return square % 8;
}

constexpr std::uint_fast8_t From(const std::uint_fast8_t file,
                                 const std::uint_fast8_t rank) {
  return 8 * rank + file;
}

}  // namespace square
}  // namespace engine

#endif
