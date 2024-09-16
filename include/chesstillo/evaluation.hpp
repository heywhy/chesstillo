#ifndef EVALUATION_HPP
#define EVALUATION_HPP

#include <tuple>

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

#define TOTAL_PHASE 24
#define TAPER_EVAL(opening, endgame, phase)                                    \
  ((opening * (256 - phase)) + (endgame * phase)) / 256

// INFO: opening & endgame weights
// order of weights should corresponding with type::piece spec.
const int kWeights[14][2] = {
    {500, 500},
    {325, 325},
    {325, 325},
    {2000, 2000},
    {975, 975},
    {70, 90},
    // bishop pair values
    {50, 50},
    // pawns structure
    {10, 20},
    {10, 20},
    {20, 20},
    {8, 10},
    {16, 10},
    {5, 10},
    // tempo
    {20, 10},
};

const float kRankBonus[] = {0, 0, 0.1, 0.3, 0.6, 1};

struct EvalState {
  Bitboard *white_pieces;
  Bitboard *black_pieces;
  Bitboard occupied_sqs;
  Bitboard check_mask;
  Bitboard pin_hv_mask;
  Bitboard pin_diag_mask;

  int phase;
  int materials[4][6];

  EvalState(Bitboard *white_pieces, Bitboard *black_pieces,
            Bitboard occupied_sqs, Bitboard check_mask, Bitboard pin_hv_mask,
            Bitboard pin_diag_mask)
      : white_pieces(white_pieces), black_pieces(black_pieces),
        occupied_sqs(occupied_sqs), check_mask(check_mask),
        pin_hv_mask(pin_hv_mask), pin_diag_mask(pin_diag_mask) {
    ComputeMaterials();
    ComputePhase();
  }

  static EvalState For(Position &position);

private:
  void ComputePhase();
  void ComputeMaterials();
};

template <enum Color side> int DoublePawns(Bitboard pawns);
template <enum Color side>
std::tuple<int, int> IsolatedPawns(Bitboard pawns, Bitboard empty_sqs);
template <enum Color side>
std::tuple<int, int> BackwardPawns(Bitboard side_pawns, Bitboard enemy_pawns);

inline std::tuple<int, int> EvalMobility(EvalState &state);
inline std::tuple<int, int> EvalMaterials(EvalState &state);
inline std::tuple<int, int> EvalPawnStructure(EvalState &state);

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
#endif
