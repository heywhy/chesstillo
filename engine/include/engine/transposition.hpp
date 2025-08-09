#ifndef ENGINE_TRANSPOSITION_HPP
#define ENGINE_TRANSPOSITION_HPP

#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <vector>

#include "move.hpp"
#include "position.hpp"
#include "threads.hpp"
#include "types.hpp"

#define MAX_TRANSPOSITION_SIZE 2097152 * sizeof(engine::TTEntry)

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
};

}  // namespace engine
#endif
