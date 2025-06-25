#ifndef TRANSPOSITION_HPP
#define TRANSPOSITION_HPP

#include <atomic>
#include <cstdint>
#include <cstdlib>
#include <vector>

#include "position.hpp"
#include "types.hpp"

#define MAX_TRANSPOSITION_SIZE 2000
#define ZOBRIST_INDEX(piece, color, square) \
  ((piece * 128) + (64 * color) + square)

struct TTEntry {
  std::uint64_t hash;
  int depth;
  int score;
  Move best_move;
  std::uint8_t age;
  NodeType node;
};

class Zobrist {
 private:
  std::uint64_t color_;
  std::uint64_t piece_sq_[6 * 2 * 64];
  std::uint64_t castling_rights_[4];
  std::uint64_t en_passant_file_[8];

  void Init();

  friend class TT;
};

class TT {
 public:
  // INFO: check if the UCI option for TT is a size or capacity.
  TT(std::uint64_t size) : size_(size), entries_(size) { zobrist_.Init(); };

  void Add(Position &position, int depth, int score, Move best_move,
           NodeType node);
  bool Probe(Position &position, TTEntry *entry);
  bool CutOff(Position &position, int depth, int alpha, int beta,
              Move *best_move, int *score);
  void Clear() noexcept;

 private:
  std::uint64_t size_;
  std::vector<std::atomic<TTEntry>> entries_;

  Zobrist zobrist_;

  std::uint64_t Hash(Position &position);

  void FreeEntry(TTEntry *entry) { free(entry); }
};
#endif
