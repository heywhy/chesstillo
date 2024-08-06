#ifndef CONSTANTS_HPP
#define CONSTANTS_HPP

#include "types.hpp"

inline constexpr Bitboard kAFile = 0x0101010101010101;
inline constexpr Bitboard kBFile = 0x0202020202020202;
inline constexpr Bitboard kGFile = 0x4040404040404040;
inline constexpr Bitboard kHFile = 0x8080808080808080;
inline constexpr Bitboard kRank1 = 0x00000000000000FF;
inline constexpr Bitboard kRank2 = 0x000000000000FF00;
inline constexpr Bitboard kRank4 = 0x00000000FF000000;
inline constexpr Bitboard kRank5 = 0x000000FF00000000;
inline constexpr Bitboard kRank7 = 0x00FF000000000000;
inline constexpr Bitboard kRank8 = 0xFF00000000000000;
inline constexpr Bitboard kA1H8Diagonal = 0x8040201008040201;
inline constexpr Bitboard kH1A8Diagonal = 0x0102040810204080;
inline constexpr Bitboard kLightSquares = 0x55AA55AA55AA55AA;
inline constexpr Bitboard kDarkSquares = 0xAA55AA55AA55AA55;
inline constexpr Bitboard kA2A7 = 0x0001010101010100;
inline constexpr Bitboard kB2G7 = 0x0040201008040200;
inline constexpr Bitboard kH1B7 = 0x0002040810204080;

#endif
