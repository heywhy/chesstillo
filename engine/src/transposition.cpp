#include <cstdint>

#include <engine/hash.hpp>
#include <engine/move_gen.hpp>
#include <engine/position.hpp>
#include <engine/square.hpp>
#include <engine/threads.hpp>
#include <engine/transposition.hpp>
#include <engine/types.hpp>

namespace engine {

TT::TT(std::size_t size) { Resize(size); };

void TT::Resize(std::size_t size) {
  if (size & (size - 1)) {
    size--;
    for (int i = 1; i < 32; i = i * 2) {
      size |= size >> i;
    }
    size++;
    size >>= 1;
  }

  if (size < sizeof(TTEntry)) {
    size_ = 0;
    return;
  }

  size_ = (size / sizeof(TTEntry)) - 1;
  entries_.resize(size_ + 1);
}

void TT::Clear() noexcept {
  std::size_t count = size_ + 1;

  for (std::size_t i = 0; i < count; i++) {
    entries_[i].hash = -1;
  }
};

void TT::Add(Position &position, int depth, int score, const Move &best_move,
             NodeType node) {
  std::uint64_t hash = position.hash_;
  TTEntry &entry = entries_[hash & size_];

  entry.spin.Lock();

  if (entry.hash == hash &&
      (entry.depth > depth || entry.age > position.halfmove_clock_)) {
    entry.spin.Unlock();
    return;
  }

  entry.hash = hash;
  entry.depth = depth;
  entry.score = score;
  entry.best_move = best_move;
  entry.age = position.halfmove_clock_;
  entry.node = node;

  entry.spin.Unlock();
};

bool TT::Probe(Position &position, TTEntry *result) {
  std::uint64_t hash = position.hash_;
  TTEntry &entry = entries_[hash & size_];

  entry.spin.Lock();

  if (entry.hash == hash) {
    result->depth = entry.depth;
    result->score = entry.score;
    result->best_move = entry.best_move;
    result->age = entry.age;
    result->node = entry.node;

    entry.spin.Unlock();

    return true;
  }

  entry.spin.Unlock();

  return false;
}

bool TT::CutOff(Position &position, int depth, int alpha, int beta,
                Move *best_move, int *score) {
  TTEntry entry;

  if (Probe(position, &entry) && entry.depth >= depth) {
    *best_move = entry.best_move;
    *score = entry.score;

    return (entry.node == NodeType::PV) ||
           (entry.node == NodeType::CUT && entry.score >= beta) ||
           (entry.node == NodeType::ALL && entry.score < alpha);
  }

  return false;
}

}  // namespace engine
