#ifndef ENGINE_HASH_HPP
#define ENGINE_HASH_HPP

#include <cstdint>

#define ZOBRIST_INDEX(piece, color, square) \
  ((piece * 128) + (64 * color) + square)

#define HASH1(to, color, piece) \
  kZobrist.piece_sq[ZOBRIST_INDEX(piece, color, to)]

#define HASH2(from, to, color, piece) \
  HASH1(from, color, piece) ^ HASH1(to, color, piece)

namespace engine {

struct Zobrist {
  std::uint64_t color;
  std::uint64_t piece_sq[6 * 2 * 64];
  std::uint64_t castling_rights[4];
  std::uint64_t en_passant_file[8];
};

extern const Zobrist kZobrist;

}  // namespace engine

#endif
