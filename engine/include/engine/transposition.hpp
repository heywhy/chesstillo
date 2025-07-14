#ifndef TRANSPOSITION_HPP
#define TRANSPOSITION_HPP

#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <random>
#include <vector>

#include "move.hpp"
#include "position.hpp"
#include "threads.hpp"
#include "types.hpp"

#define MAX_TRANSPOSITION_SIZE 2097152 * sizeof(engine::TTEntry)
#define ZOBRIST_INDEX(piece, color, square) \
  ((piece * 128) + (64 * color) + square)

namespace engine {

struct TTEntry {
  std::uint64_t hash = -1;
  int depth;
  int score;
  Move best_move;
  std::uint8_t age;
  NodeType node;

  SpinLock spin;

  TTEntry() = default;

  TTEntry(const TTEntry &entry) {
    hash = entry.hash;
    depth = entry.hash;
    score = entry.score;
    best_move = entry.best_move;
    age = entry.age;
    node = entry.node;
  }
};

struct Zobrist {
  std::uint64_t color;
  std::uint64_t piece_sq[6 * 2 * 64];
  std::uint64_t castling_rights[4];
  std::uint64_t en_passant_file[8];

  void Init();

 private:
  std::mt19937_64 rng_;
};

class TT {
 public:
  // INFO: check if the UCI option for TT is a size or capacity.
  TT(std::size_t size);

  void Add(Position &position, int depth, int score, const Move &best_move,
           NodeType node);
  bool Probe(Position &position, TTEntry *entry);
  bool CutOff(Position &position, int depth, int alpha, int beta,
              Move *best_move, int *score);
  void Clear() noexcept;
  void Resize(std::size_t new_size);

 private:
  std::size_t size_;
  std::vector<TTEntry> entries_;

  Zobrist zobrist_;

  std::uint64_t Hash(Position &position);
};

}  // namespace engine
#endif
