#include <cstdint>
#include <utility>

#include <chesstillo/move_gen.hpp>
#include <chesstillo/position.hpp>
#include <chesstillo/transposition.hpp>
#include <chesstillo/types.hpp>

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
        piece_sq_[ZOBRIST_INDEX(piece, color, sq)] = Rand();
      }
    }
  }

  for (int i = 0; i < 4; i++) {
    castling_rights_[i] = Rand();
  }

  for (int i = 0; i < 8; i++) {
    en_passant_file_[i] = Rand();
  }

  color_ = Rand();
}

void TT::Clear() noexcept {
  for (uint64_t i = 0; i < size_; i++) {
    entries_[i].hash = -1;
  }
};

uint64_t TT::Hash(Position &position) {
  uint64_t hash = 0;

  for (int color = 0; color < COLOR; color++) {
    Bitboard *pieces = position.Pieces(static_cast<Color>(color));

    for (int piece = 0; piece < PIECES; piece++) {
      Bitboard piece_bb = pieces[piece];

      BITLOOP(piece_bb) {
        uint64_t index =
            zobrist_.piece_sq_[ZOBRIST_INDEX(piece, color, LOOP_INDEX)];

        hash ^= index;
      }
    }
  }

  if (position.turn_ == BLACK) {
    hash ^= zobrist_.color_;
  }

  return hash;
};

void TT::Add(Position &position, int depth, int score, Move best_move,
             NodeType node) {
  uint64_t hash = Hash(position);
  TTEntry *entry = &entries_[hash & size_];

  if ((entry->hash == hash && entry->depth > depth) ||
      (entry->age > position.halfmove_clock_)) {
    return;
  }

  entry->hash = hash;
  entry->depth = depth;
  entry->score = score;
  entry->best_move = std::move(best_move);
  entry->age = position.halfmove_clock_;
  entry->node = node;
};

bool TT::Probe(Position &position, TTEntry *result) {
  uint64_t hash = Hash(position);
  TTEntry *entry = &entries_[hash & size_];

  if (entry->hash == hash) {
    // copy the entry to avoid data race
    *result = *entry;

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
           (entry.node == ALL && entry.score <= alpha);
  }

  return false;
}
