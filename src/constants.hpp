#include "types.hpp"

#ifndef CONSTANTS_HPP

#define CONSTANTS_HPP

static const Bitboard kAFile = 0x0101010101010101;
static const Bitboard kBFile = 0x0202020202020202;
static const Bitboard kHFile = 0x8080808080808080;
static const Bitboard kRank1 = 0xFF00;
static const Bitboard kRank4 = 0x00000000FF000000;
static const Bitboard kRank5 = 0x000000FF00000000;
static const Bitboard kRank8 = 0x00FF000000000000;
static const Bitboard kA1H8Diagonal = 0x8040201008040201;
static const Bitboard kH1A8Diagonal = 0x0102040810204080;
static const Bitboard kLightSquares = 0x55AA55AA55AA55AA;
static const Bitboard kDarkSquares = 0xAA55AA55AA55AA55;

#endif
