#include <chesstillo/board.hpp>
#include <chesstillo/constants.hpp>
#include <chesstillo/move_gen.hpp>
#include <chesstillo/types.hpp>

const magic_bits::Attacks kSlidingAttacks;

consteval AttackMaps InitAttackMaps() {
  AttackMaps m{};

  for (int i = 0; i < 64; i++) {
    Bitboard square = BITBOARD_FOR_SQUARE(i);

    m[KING][i] = KING_ATTACKS(square);
    m[KNIGHT][i] = KNIGHT_ATTACKS(square);
    m[ROOK][i] = RankMask(i) ^ FileMask(i);
    m[BISHOP][i] = DiagonalMask(i) ^ AntiDiagonalMask(i);
  }

  return m;
}

constexpr AttackMaps kAttackMaps = InitAttackMaps();

consteval CheckBetween InitCheckBetween() {
  CheckBetween m{};

  for (int i = 0; i < 64; i++) {
    for (int j = 0; j < 64; j++) {
      m[ROOK][i][j] = SquaresInBetween(i, j);
      m[BISHOP][i][j] = SquaresInBetween(i, j);
    }
  }

  return m;
}

constexpr CheckBetween kCheckBetween = InitCheckBetween();
