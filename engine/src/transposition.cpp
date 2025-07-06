#include <cstdint>
#include <utility>

#include <engine/move_gen.hpp>
#include <engine/position.hpp>
#include <engine/square.hpp>
#include <engine/transposition.hpp>
#include <engine/types.hpp>

namespace engine {

// INFO: copied from cpw_engine transposition
uint64_t Rand() {
  static uint64_t N = 1;

  N = N * 1103515245 + 12345;

  return N;
}

void Zobrist::Init() {
  for (int piece = 0; piece < PIECES; piece++) {
    for (int color = 0; color < COLOR; color++) {
      for (int sq = 0; sq < 64; sq++) {
        piece_sq[ZOBRIST_INDEX(piece, color, sq)] = Rand();
      }
    }
  }

  for (int i = 0; i < 4; i++) {
    castling_rights[i] = Rand();
  }

  for (int i = 0; i < 8; i++) {
    en_passant_file[i] = Rand();
  }

  color = Rand();
}

TTEntry::TTEntry() : best_move(kNullMove) {}

TTEntry &TTEntry::operator=(const TTEntry &entry) {
  hash = entry.hash;
  depth = entry.depth;
  score = entry.score;
  best_move = entry.best_move;
  age = entry.age;
  node = entry.node;

  return *this;
}

TT::TT(std::size_t size) : size_(size), entries_(size) { zobrist_.Init(); };

void TT::Clear() noexcept {
  for (uint64_t i = 0; i < size_; i++) {
    TTEntry &entry = entries_[i];

    entry.hash = -1;
  }
};

uint64_t TT::Hash(Position &position) {
  uint64_t hash = 0;

  for (int color = 0; color < COLOR; color++) {
    const PieceList &pieces = position.Pieces(static_cast<Color>(color));

    for (int piece = 0; piece < PIECES; piece++) {
      Bitboard piece_bb = pieces[piece];

      BITLOOP(piece_bb) {
        uint64_t index =
            zobrist_.piece_sq[ZOBRIST_INDEX(piece, color, LOOP_INDEX)];

        hash ^= index;
      }
    }
  }

  if (position.turn_ == BLACK) {
    hash ^= zobrist_.color;
  }

  return hash;
};

void TT::Add(Position &position, int depth, int score, const Move &best_move,
             NodeType node) {
  uint64_t hash = Hash(position);
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
  uint64_t hash = Hash(position);
  TTEntry &entry = entries_[hash & size_];

  if (entry.hash == hash) {
    *result = entry;

    return true;
  }

  return false;
}

bool TT::CutOff(Position &position, int depth, int alpha, int beta,
                Move *best_move, int *score) {
  TTEntry entry;

  if (Probe(position, &entry) && entry.depth >= depth) {
    *best_move = entry.best_move;
    *score = entry.score;

    return (entry.node == PV) || (entry.node == CUT && entry.score >= beta) ||
           (entry.node == ALL && entry.score < alpha);
  }

  return false;
}

}  // namespace engine
