#ifndef CONSTANTS_HPP
#define CONSTANTS_HPP

#include <array>
#include <magic_bits.hpp>

#include "types.hpp"

namespace engine {

inline constexpr Bitboard kEmpty = 0;
inline constexpr Bitboard kUniverse = -1;
inline constexpr Bitboard kAFile = 0x0101010101010101;
inline constexpr Bitboard kBFile = 0x0202020202020202;
inline constexpr Bitboard kGFile = 0x4040404040404040;
inline constexpr Bitboard kHFile = 0x8080808080808080;
inline constexpr Bitboard kRank1 = 0x00000000000000FF;
inline constexpr Bitboard kRank2 = 0x000000000000FF00;
inline constexpr Bitboard kRank3 = 0x0000000000FF0000;
inline constexpr Bitboard kRank4 = 0x00000000FF000000;
inline constexpr Bitboard kRank5 = 0x000000FF00000000;
inline constexpr Bitboard kRank6 = 0x0000FF0000000000;
inline constexpr Bitboard kRank7 = 0x00FF000000000000;
inline constexpr Bitboard kRank8 = 0xFF00000000000000;
inline constexpr Bitboard kA1H8Diagonal = 0x8040201008040201;
inline constexpr Bitboard kH1A8Diagonal = 0x0102040810204080;
inline constexpr Bitboard kLightSquares = 0x55AA55AA55AA55AA;
inline constexpr Bitboard kDarkSquares = 0xAA55AA55AA55AA55;
inline constexpr Bitboard kA2A7 = 0x0001010101010100;
inline constexpr Bitboard kB2G7 = 0x0040201008040200;
inline constexpr Bitboard kH1B7 = 0x0002040810204080;
inline constexpr Bitboard kKingSide = 0xF0F0F0F0F0F0F0F0;
inline constexpr Bitboard kQueenSide = 0x0F0F0F0F0F0F0F0F;
inline constexpr Bitboard kCastleKingSidePath = 0x6000000000000060;
inline constexpr Bitboard kCastleQueenSidePath = 0x0C0000000000000C;

inline constexpr Bitboard kCenter = 0x0000001818000000;
inline constexpr Bitboard kExtendedCenter = 0x00003C3C3C3C0000;

typedef std::array<std::array<Bitboard, 64>, 4> AttackMaps;
typedef std::array<std::array<std::array<Bitboard, 64>, 64>, 2> CheckBetween;

extern const AttackMaps kAttackMaps;
extern const CheckBetween kCheckBetween;
extern const magic_bits::Attacks kSlidingAttacks;

consteval AttackMaps InitAttackMaps();
consteval CheckBetween InitCheckBetween();

}  // namespace engine

#endif
