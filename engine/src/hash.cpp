#include <cstdint>

#include "engine/hash.hpp"
#include "engine/types.hpp"

namespace engine {

consteval std::uint64_t Rand(std::uint64_t &N) {
  N = N * 1103515245 + 12345;

  return N;
}

consteval Zobrist InitZobrist() {
  Zobrist z{};
  std::uint64_t N = 1;

  for (int piece = 0; piece < PIECES; piece++) {
    for (int color = 0; color < COLOR; color++) {
      for (int sq = 0; sq < 64; sq++) {
        z.piece_sq[ZOBRIST_INDEX(piece, color, sq)] = Rand(N);
      }
    }
  }

  for (int i = 0; i < 4; i++) {
    z.castling_rights[i] = Rand(N);
  }

  for (int i = 0; i < 8; i++) {
    z.en_passant_file[i] = Rand(N);
  }

  z.color = Rand(N);

  return z;
}

constexpr Zobrist kZobrist = InitZobrist();

}  // namespace engine
