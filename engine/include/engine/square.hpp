#ifndef ENGINE_SQUARE_HPP
#define ENGINE_SQUARE_HPP

#include <bit>
#include <cassert>

#include "constants.hpp"
#include "types.hpp"

namespace engine {
namespace square {

constexpr Bitboard BB(const int square) {
  assert(0 <= square && square <= 63);

  return static_cast<Bitboard>(1) << square;
}

template <typename T>
constexpr int Index(const T bb) {
  return std::countr_zero(bb);
}

constexpr int MIndex(const Bitboard bb) { return 63 - std::countl_zero(bb); }

constexpr Bitboard Occupancy(const Bitboard* pieces) {
  return pieces[KING] | pieces[QUEEN] | pieces[BISHOP] | pieces[KNIGHT] |
         pieces[ROOK] | pieces[PAWN];
}

constexpr Bitboard Occupancy(const PieceList& pieces) {
  return Occupancy(pieces.data());
}

constexpr int Rank(const int square) { return square / 8; }

constexpr int File(const int square) { return square % 8; }

constexpr int From(const int file, const int rank) { return 8 * rank + file; }

constexpr Bitboard RankMask(int square) { return kRank1 << (square & 56); }

constexpr Bitboard FileMask(int square) { return kAFile << (square & 7); }

constexpr Bitboard DiagonalMask(int square) {
  int diagonal = (square & 7) - (square >> 3);

  return diagonal >= 0 ? kA1H8Diagonal >> diagonal * 8
                       : kA1H8Diagonal << -diagonal * 8;
}

constexpr Bitboard AntiDiagonalMask(int square) {
  int diagonal = 7 - (square & 7) - (square >> 3);

  return diagonal >= 0 ? kH1A8Diagonal >> diagonal * 8
                       : kH1A8Diagonal << -diagonal * 8;
}

constexpr Bitboard InBetween(int from, int to) {
  const Bitboard M = -1;
  Bitboard btwn, line, rank, file;

  to = to >= 64 ? 63 : to;
  from = from >= 64 ? 63 : from;

  btwn = (M << from) ^ (M << to);
  file = (to & 7) - (from & 7);
  rank = ((to | 7) - from) >> 3;
  line = ((file & 7) - 1) & kA2A7;

  line += 2 * (((rank & 7) - 1) >> 58);
  line += (((rank - file) & 15) - 1) & kB2G7;
  line += (((rank + file) & 15) - 1) & kH1B7;
  line *= btwn & -btwn;

  return line & btwn;
}

}  // namespace square
}  // namespace engine

#endif
