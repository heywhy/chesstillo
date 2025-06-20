#ifndef EVALUATION_HPP
#define EVALUATION_HPP

#include <tuple>
#include <utility>

#include "position.hpp"
#include "types.hpp"

#define DIFF 2
#define SUM 3

#define BISHOP_PAIR 6
#define DOUBLED_PAWNS 7
#define ISOLATED_PAWNS 8
#define ISOLATED_AND_OPEN_PAWNS 9
#define BACKWARD_PAWNS 10
#define BACKWARD_AND_OPEN_PAWNS 11
#define CANDIDATE_PAWNS 12
#define TEMPO 13
#define KNIGHT_MOBILITY 14
#define BISHOP_MOBILITY 15
#define ROOK_MOBILITY 16
#define CLOSED_FILE 17
#define SEMI_OPEN_FILE 18
#define SEMI_OPEN_FILE_ADJ_ENEMY_KING 19
#define SEMI_OPEN_FILE_SAME_ENEMY_KING 20
#define OPEN_FILE 21
#define OPEN_FILE_ADJ_ENEMY_KING 22
#define OPEN_FILE_SAME_ENEMY_KING 23
#define ROOK_ON_7th 24
#define QUEEN_ON_7th 25

#define TOTAL_PHASE 24
#define TAPER_EVAL(opening, endgame, phase) \
  ((opening * (256 - phase)) + (endgame * phase)) / 256

// INFO: opening & endgame weights
// order of weights should corresponding with type::piece spec.

namespace engine {

extern const int kWeights[26][2];
extern const float kRankBonus[6];

struct EvalState {
  Bitboard *white_pieces;
  Bitboard *black_pieces;
  Bitboard occupied_sqs;
  Bitboard check_mask;
  Bitboard pin_hv_mask;
  Bitboard pin_diag_mask;

  int phase;
  int materials[4][6];
  Bitboard attack_map[2];

  EvalState(Bitboard *white_pieces, Bitboard *black_pieces,
            Bitboard occupied_sqs, Bitboard check_mask, Bitboard pin_hv_mask,
            Bitboard pin_diag_mask)
      : white_pieces(white_pieces),
        black_pieces(black_pieces),
        occupied_sqs(occupied_sqs),
        check_mask(check_mask),
        pin_hv_mask(pin_hv_mask),
        pin_diag_mask(pin_diag_mask) {
    ComputeMaterials();
    ComputePhase();
    ComputeAttackMap<WHITE>();
    ComputeAttackMap<BLACK>();
  }

  static EvalState For(Position &position);

 private:
  void ComputePhase();
  void ComputeMaterials();
  template <enum Color side>
  void ComputeAttackMap();
};

enum Phase : uint8_t { OPENING, ENDGAME };

template <enum Color side>
int DoublePawns(Bitboard pawns);
template <enum Color side>
std::pair<int, int> IsolatedPawns(Bitboard pawns, Bitboard empty_sqs);
template <enum Color side>
std::pair<int, int> BackwardPawns(Bitboard side_pawns, Bitboard enemy_pawns);

template <enum Color side>
int ClosedFiles(EvalState &state);
template <enum Color side>
std::tuple<int, int, int> SemiOpenFiles(EvalState &state);
template <enum Color side>
std::tuple<int, int, int> OpenFiles(EvalState &state);

template <enum Color side, enum Piece piece>
int Rank7(EvalState &state);
template <enum Color side>
int KingDistance(EvalState &state);

template <enum Color>
int RooksMobility(EvalState &state);
template <enum Color>
int BishopsMobility(EvalState &state);
template <enum Color>
int KnightsMobility(EvalState &state);
template <enum Color>
std::pair<float, float> PassedPawns(EvalState &state);

template <enum Color>
int KingPosition(EvalState &state);

std::pair<int, int> EvalPieces(EvalState &state);
int EvalKingDistance(EvalState &state);
std::pair<int, int> EvalMobility(EvalState &state);
std::pair<int, int> EvalOpenFile(EvalState &state);
std::pair<int, int> EvalRank7(EvalState &state);
std::pair<int, int> EvalMaterials(EvalState &state);
std::pair<float, float> EvalPassedPawns(EvalState &state);
std::pair<float, float> EvalPawnStructure(EvalState &state);
int EvalKingPosition(EvalState &state);

int Evaluate(Position &position);

inline int PieceValue(Piece piece) {
  switch (piece) {
    case KING:
      return kWeights[KING][0];
    case QUEEN:
      return kWeights[QUEEN][0];
    case ROOK:
      return kWeights[ROOK][0];
    case KNIGHT:
      return kWeights[KNIGHT][0];
    case BISHOP:
      return kWeights[BISHOP][0];
    case PAWN:
      return kWeights[PAWN][0];
    default:
      return 0;
  }
}

}  // namespace engine

#endif
