#include "engine/constants.hpp"
#include "engine/move_gen.hpp"
#include "engine/square.hpp"
#include "engine/types.hpp"

namespace engine {

const magic_bits::Attacks kSlidingAttacks;

consteval AttackMaps InitAttackMaps() {
  AttackMaps m{};

  for (int i = 0; i < 64; i++) {
    Bitboard square = square::BB(i);

    m[KING][i] = KING_ATTACKS(square);
    m[KNIGHT][i] = KNIGHT_ATTACKS(square);
    m[ROOK][i] = square::RankMask(i) ^ square::FileMask(i);
    m[BISHOP][i] = square::DiagonalMask(i) ^ square::AntiDiagonalMask(i);
  }

  return m;
}

constexpr AttackMaps kAttackMaps = InitAttackMaps();

consteval CheckBetween InitCheckBetween() {
  CheckBetween m{};

  for (int i = 0; i < 64; i++) {
    for (int j = 0; j < 64; j++) {
      m[ROOK][i][j] = square::InBetween(i, j);
      m[BISHOP][i][j] = square::InBetween(i, j);
    }
  }

  return m;
}

constexpr CheckBetween kCheckBetween = InitCheckBetween();

}  // namespace engine
