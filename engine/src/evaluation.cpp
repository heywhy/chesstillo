#include <array>
#include <bit>
#include <cmath>
#include <cstdlib>
#include <tuple>
#include <utility>

#include <engine/board.hpp>
#include <engine/constants.hpp>
#include <engine/evaluation.hpp>
#include <engine/fill.hpp>
#include <engine/move_gen.hpp>
#include <engine/position.hpp>
#include <engine/square.hpp>
#include <engine/types.hpp>

namespace engine {

const int kWeights[26][2] = {{500, 500},
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
                             // knight mobility
                             {4, 4},
                             // bishop mobility
                             {5, 5},
                             // rook mobility
                             {2, 4},
                             // file
                             {-10, -10},
                             {0, 0},
                             {10, 0},
                             {20, 0},
                             {10, 10},
                             {20, 10},
                             {30, 10},
                             // 7th rank
                             {20, 40},
                             {10, 20}};

const float kRankBonus[] = {0, 0, 0.1, 0.3, 0.6, 1};

void EvalState::ComputeMaterials() {
  for (int i = 0; i < PIECES; i++) {
    Piece piece = static_cast<Piece>(i);

    materials[WHITE][piece] = std::popcount(white_pieces[piece]);
    materials[BLACK][piece] = std::popcount(black_pieces[piece]);

    materials[DIFF][piece] = materials[WHITE][piece] - materials[BLACK][piece];
    materials[SUM][piece] = materials[WHITE][piece] + materials[BLACK][piece];
  }
}

void EvalState::ComputePhase() {
  int score = TOTAL_PHASE - (4 * materials[SUM][QUEEN]) -
              (2 * materials[SUM][ROOK]) - (materials[SUM][BISHOP]) -
              (materials[SUM][KNIGHT]);

  phase = (score * 256 + (TOTAL_PHASE / 2)) / TOTAL_PHASE;
}

// TODO: maybe apply pin masks to filter what pieces attacks are counted
template <enum Color side>
void EvalState::ComputeAttackMap() {
  Bitboard *side_pieces;

  if constexpr (side == WHITE) {
    side_pieces = white_pieces;
  } else {
    side_pieces = black_pieces;
  }

  Bitboard queens_and_bishops = side_pieces[QUEEN] | side_pieces[BISHOP];
  Bitboard queens_and_rooks = side_pieces[QUEEN] | side_pieces[ROOK];

  attack_map[side] = kEmpty;
  attack_map[side] |= PawnTargets<side>(side_pieces[PAWN]) |
                      KNIGHT_ATTACKS(side_pieces[KNIGHT]) |
                      KING_ATTACKS(side_pieces[KING]);

  BITLOOP(queens_and_bishops) {
    attack_map[side] |= kSlidingAttacks.Bishop(occupied_sqs, LOOP_INDEX);
  }

  BITLOOP(queens_and_rooks) {
    attack_map[side] |= kSlidingAttacks.Rook(occupied_sqs, LOOP_INDEX);
  }
}

EvalState EvalState::For(Position &position) {
  PieceList &white_pieces = position.Pieces(WHITE);
  PieceList &black_pieces = position.Pieces(BLACK);
  Bitboard occupied_sqs = position.board_.occupied_sqs;

  // TODO: maybe move position mask computation into the position
  // class so as to avoid double computation of these masks.
  Bitboard check_mask = CheckMask(position);
  auto [pin_hv_mask, pin_diag_mask] = PinMask(position);

  return {white_pieces.data(), black_pieces.data(), occupied_sqs,
          check_mask,          pin_hv_mask,         pin_diag_mask};
}

// TODO: Position table, Pattern, King, Passed Pawn
// convert score to float
int Evaluate(Position &position) {
  EvalState state = EvalState::For(position);
  int side_to_move = position.turn_ == WHITE ? 1 : -1;

  auto [opening_pieces, endgame_pieces] = EvalPieces(state);
  auto [opening_materials, endgame_materials] = EvalMaterials(state);
  auto [opening_pawn_structure, endgame_pawn_structure] =
      EvalPawnStructure(state);
  auto opening_king_position = EvalKingPosition(state);

  auto [opening_tempo, endgame_tempo] = std::make_pair(
      kWeights[TEMPO][0] * side_to_move, kWeights[TEMPO][1] * side_to_move);

  auto [opening_passed_pawns, endgame_passed_pawns] = EvalPassedPawns(state);

  int opening = opening_tempo + opening_materials + opening_pieces -
                opening_pawn_structure + opening_king_position +
                opening_passed_pawns;
  int endgame = endgame_tempo + endgame_materials + endgame_pieces -
                endgame_pawn_structure + endgame_passed_pawns;

  return TAPER_EVAL(opening, endgame, state.phase);
}

std::pair<int, int> EvalMaterials(EvalState &state) {
  int scores[2];
  int white_bishop_pair = 0;
  int black_bishop_pair = 0;

  int pawns = state.materials[DIFF][PAWN];
  int kings = state.materials[DIFF][KING];
  int queens = state.materials[DIFF][QUEEN];
  int rooks = state.materials[DIFF][ROOK];
  int bishops = state.materials[DIFF][BISHOP];
  int knights = state.materials[DIFF][KNIGHT];

  Bitboard white_bishops = state.white_pieces[BISHOP];
  Bitboard black_bishops = state.black_pieces[BISHOP];

  white_bishop_pair =
      kLightSquares & white_bishops && kDarkSquares & white_bishops;

  black_bishop_pair =
      kLightSquares & black_bishops && kDarkSquares & black_bishops;

  for (int i = 0; i < 2; i++) {
    scores[i] =
        (kWeights[PAWN][i] * pawns) + (kWeights[KING][i] * kings) +
        (kWeights[QUEEN][i] * queens) + (kWeights[ROOK][i] * rooks) +
        (kWeights[BISHOP][i] * bishops) + (kWeights[KNIGHT][i] * knights) +
        (kWeights[BISHOP_PAIR][i] * (white_bishop_pair - black_bishop_pair));
  }

  return std::make_pair(scores[0], scores[1]);
}

// TODO: maybe include the "candidate" property from fruits/TOGA
std::pair<float, float> EvalPawnStructure(EvalState &state) {
  float scores[2];
  Bitboard white_pawns = state.white_pieces[PAWN];
  Bitboard black_pawns = state.black_pieces[PAWN];
  Bitboard empty_sqs = ~(white_pawns | black_pawns);

  int double_pawns =
      DoublePawns<WHITE>(white_pawns) - DoublePawns<BLACK>(black_pawns);

  auto [white_isolated_pawns, white_open_isolated_pawns] =
      IsolatedPawns<WHITE>(white_pawns, empty_sqs);
  auto [black_isolated_pawns, black_open_isolated_pawns] =
      IsolatedPawns<BLACK>(black_pawns, empty_sqs);

  int isolated_pawns = white_isolated_pawns - black_isolated_pawns;
  int open_isolated_pawns =
      white_open_isolated_pawns - black_open_isolated_pawns;

  auto [white_backward_pawns, white_open_backward_pawns] =
      BackwardPawns<WHITE>(white_pawns, black_pawns);
  auto [black_backward_pawns, black_open_backward_pawns] =
      BackwardPawns<BLACK>(black_pawns, white_pawns);

  int backward_pawns = white_backward_pawns - black_backward_pawns;
  int open_backward_pawns =
      white_open_backward_pawns - black_open_backward_pawns;

  for (int i = 0; i < 2; i++) {
    scores[i] = (kWeights[DOUBLED_PAWNS][i] * double_pawns) +
                (kWeights[ISOLATED_PAWNS][i] * isolated_pawns) +
                (kWeights[BACKWARD_PAWNS][i] * backward_pawns) +
                (kWeights[ISOLATED_AND_OPEN_PAWNS][i] * open_isolated_pawns) +
                (kWeights[BACKWARD_AND_OPEN_PAWNS][i] * open_backward_pawns);
  }

  return std::make_pair(scores[0], scores[1]);
}

// TODO: knight outpost
std::pair<int, int> EvalPieces(EvalState &state) {
  int king_distance = EvalKingDistance(state);
  auto [opening_mobility, endgame_mobility] = EvalMobility(state);
  auto [opening_open_line, endgame_open_line] = EvalOpenFile(state);
  auto [opening_7th_rank, endgame_7th_rank] = EvalRank7(state);

  int opening =
      opening_mobility + opening_open_line + opening_7th_rank + king_distance;
  int endgame =
      endgame_mobility + endgame_open_line + endgame_7th_rank + king_distance;

  return std::make_pair(opening, endgame);
}

std::pair<float, float> EvalPassedPawns(EvalState &state) {
  auto [opening_white_passed_pawns, endgame_white_passed_pawns] =
      PassedPawns<WHITE>(state);

  auto [opening_black_passed_pawns, endgame_black_passed_pawns] =
      PassedPawns<BLACK>(state);

  float opening = opening_white_passed_pawns - opening_black_passed_pawns;
  float endgame = endgame_white_passed_pawns - endgame_black_passed_pawns;

  return std::make_pair(opening, endgame);
}

int EvalKingPosition(EvalState &state) {
  return -(KingPosition<WHITE>(state) - KingPosition<BLACK>(state));
}

// TODO: consider the castling rights while looking at pawn shelter
template <enum Color side>
int KingPosition(EvalState &state) {
  Bitboard king_bb;
  Bitboard storm_area;
  Bitboard *side_pieces;
  Bitboard *enemy_pieces;
  Bitboard (*file_fill)(Bitboard);

  if constexpr (side == WHITE) {
    file_fill = NorthFill;
    side_pieces = state.white_pieces;
    enemy_pieces = state.black_pieces;
    king_bb = state.white_pieces[KING];

    storm_area = kRank3 | kRank4 | kRank5;
  } else {
    file_fill = SouthFill;
    side_pieces = state.black_pieces;
    enemy_pieces = state.white_pieces;
    king_bb = state.black_pieces[KING];

    storm_area = kRank4 | kRank5 | kRank6;
  }

  if (!(enemy_pieces[QUEEN] &&
        (enemy_pieces[BISHOP] | enemy_pieces[KNIGHT] | enemy_pieces[ROOK]))) {
    return 0;
  }

  Bitboard king_file = file_fill(king_bb);
  Bitboard shelter_sqs =
      (PushPawn<side>(king_bb) | PawnTargets<side, EAST>(king_bb) |
       PawnTargets<side, WEST>(king_bb));

  Bitboard shelter_files = file_fill(shelter_sqs);

  // Pawn shelter
  int shelter_penalty = 0;

  Bitboard shelter_pawns = shelter_files & side_pieces[PAWN];

  BITLOOP(shelter_pawns) {
    int distance;
    int square = LOOP_INDEX;
    int rank = square::Rank(square);
    Bitboard bb = square::BB(square);

    if constexpr (side == WHITE) {
      distance = 8 - rank;
    } else {
      distance = 7 - std::abs(rank - 8);
    }

    int penalty = 36 - std::pow(distance, 2);

    if (bb & king_file) {
      penalty *= 2;
    }

    shelter_sqs ^= FileFill(bb) & shelter_sqs;
    shelter_penalty += penalty;
  };

  BITLOOP(shelter_sqs) {
    int penalty = 36;
    Bitboard bb = square::BB(LOOP_INDEX);

    if (bb & king_file) {
      penalty *= 2;
    }

    shelter_penalty += penalty;
  }

  if (shelter_penalty == 0) {
    shelter_penalty = -11;
  }

  // Piece storm
  int hostile_pawns_penalty = 0;
  Bitboard hostile_pawns = shelter_files & storm_area & enemy_pieces[PAWN];

  BITLOOP(hostile_pawns) {
    int square = LOOP_INDEX;
    Bitboard bb = square::BB(square);

    // clang-format off
    if constexpr (side == WHITE) {
      if (bb & storm_area & kRank3) hostile_pawns_penalty += 60;
      if (bb & storm_area & kRank4) hostile_pawns_penalty += 30;
      if (bb & storm_area & kRank5) hostile_pawns_penalty += 10;
    } else {
      if (bb & storm_area & kRank6) hostile_pawns_penalty += 60;
      if (bb & storm_area & kRank5) hostile_pawns_penalty += 30;
      if (bb & storm_area & kRank4) hostile_pawns_penalty += 10;
    }
    // clang-format on
  }

  // Piece attack
  int attackers_value = 0;
  Bitboard attackers = kEmpty;
  Bitboard enemy_king_adjacents = KING_ATTACKS(enemy_pieces[KING]);
  Bitboard enemy_king_adjacents_x2 =
      enemy_king_adjacents | KING_ATTACKS(enemy_king_adjacents);
  Bitboard bishop_and_queen = side_pieces[BISHOP] | side_pieces[QUEEN];

  BITLOOP(enemy_pieces[KNIGHT]) {
    int square = LOOP_INDEX;
    Bitboard targets = kAttackMaps[KNIGHT][square];

    if (targets & enemy_king_adjacents || enemy_king_adjacents_x2 & targets) {
      Bitboard bb = square::BB(square);

      attackers_value += 1;
      attackers |= bb;
    }
  }

  BITLOOP(bishop_and_queen) {
    int square = LOOP_INDEX;
    Bitboard targets = kSlidingAttacks.Bishop(state.occupied_sqs, square);

    if (kAttackMaps[BISHOP][square] & enemy_king_adjacents ||
        enemy_king_adjacents_x2 & targets) {
      Bitboard bb = square::BB(square);

      attackers_value += bb & side_pieces[BISHOP] ? 1 : 4;
      attackers |= bb;
    }
  }

  Bitboard rook_and_queen = side_pieces[ROOK] | side_pieces[QUEEN];

  BITLOOP(rook_and_queen) {
    int square = LOOP_INDEX;
    Bitboard bb = square::BB(square);
    Bitboard targets = kSlidingAttacks.Rook(state.occupied_sqs, square);

    if (kAttackMaps[ROOK][square] & enemy_king_adjacents ||
        enemy_king_adjacents_x2 & targets) {
      attackers_value += bb & side_pieces[ROOK] ? 2 : 4;
      attackers |= bb;
    }
  }

  static std::array<float, 8> PA_WEIGHTS{0,    0,    0.5,  0.75,
                                         0.88, 0.94, 0.97, 0.99};

  int attackers_count = std::popcount(attackers);
  int attackers_score = 20 * attackers_value * PA_WEIGHTS[attackers_count];

  return shelter_penalty + hostile_pawns_penalty - attackers_score;
}

std::pair<int, int> EvalMobility(EvalState &state) {
  int scores[2];

  for (int i = 0; i < 2; i++) {
    int rooks_mobility =
        kWeights[ROOK_MOBILITY][i] *
        (RooksMobility<WHITE>(state) - RooksMobility<BLACK>(state));

    int bishop_mobility =
        kWeights[BISHOP_MOBILITY][i] *
        (BishopsMobility<WHITE>(state) - BishopsMobility<BLACK>(state));

    int knight_mobility =
        kWeights[KNIGHT_MOBILITY][i] *
        (KnightsMobility<WHITE>(state) - KnightsMobility<BLACK>(state));

    scores[i] = rooks_mobility + bishop_mobility + knight_mobility;
  }

  return std::make_pair(scores[0], scores[1]);
}

std::pair<int, int> EvalOpenFile(EvalState &state) {
  int scores[2];

  int closed_files = ClosedFiles<WHITE>(state) - ClosedFiles<BLACK>(state);

  // semi-open files
  auto [white_semi_open_files, white_semi_open_files_adj_enemy_king,
        white_semi_open_files_same_enemy_king] = SemiOpenFiles<WHITE>(state);

  auto [black_semi_open_files, black_semi_open_files_adj_enemy_king,
        black_semi_open_files_same_enemy_king] = SemiOpenFiles<BLACK>(state);

  int semi_open_files = white_semi_open_files - black_semi_open_files;

  int semi_open_files_adj_enemy_king = white_semi_open_files_adj_enemy_king -
                                       black_semi_open_files_adj_enemy_king;

  int semi_open_files_same_enemy_king = white_semi_open_files_same_enemy_king -
                                        black_semi_open_files_same_enemy_king;

  // open files
  auto [white_open_files, white_open_files_adj_enemy_king,
        white_open_files_same_enemy_king] = OpenFiles<WHITE>(state);

  auto [black_open_files, black_open_files_adj_enemy_king,
        black_open_files_same_enemy_king] = OpenFiles<BLACK>(state);

  int open_files = white_open_files - black_open_files;

  int open_files_adj_enemy_king =
      white_open_files_adj_enemy_king - black_open_files_adj_enemy_king;

  int open_files_same_enemy_king =
      white_open_files_same_enemy_king - black_open_files_same_enemy_king;

  for (int i = 0; i < 2; i++) {
    scores[i] =
        (kWeights[CLOSED_FILE][i] * closed_files) +
        (kWeights[SEMI_OPEN_FILE][i] * semi_open_files) +
        (kWeights[SEMI_OPEN_FILE_ADJ_ENEMY_KING][i] *
         semi_open_files_adj_enemy_king) +
        (kWeights[SEMI_OPEN_FILE_SAME_ENEMY_KING][i] *
         semi_open_files_same_enemy_king) +
        (kWeights[OPEN_FILE][i] * open_files) +
        (kWeights[OPEN_FILE_ADJ_ENEMY_KING][i] * open_files_adj_enemy_king) +
        (kWeights[OPEN_FILE_SAME_ENEMY_KING][i] * open_files_same_enemy_king);
  }

  return std::make_pair(scores[0], scores[1]);
}

std::pair<int, int> EvalRank7(EvalState &state) {
  int scores[2];

  for (int i = 0; i < 2; i++) {
    int rooks_on_7th = (kWeights[ROOK_ON_7th][i] * (Rank7<WHITE, ROOK>(state) -
                                                    Rank7<BLACK, ROOK>(state)));

    int queens_on_7th =
        (kWeights[QUEEN_ON_7th][i] *
         (Rank7<WHITE, QUEEN>(state) - Rank7<BLACK, QUEEN>(state)));

    scores[i] = rooks_on_7th + queens_on_7th;
  }

  return std::make_pair(scores[0], scores[1]);
}

int EvalKingDistance(EvalState &state) {
  return KingDistance<WHITE>(state) - KingDistance<BLACK>(state);
}

template <enum Color side>
int DoublePawns(Bitboard pawns) {
  int count = 0;
  constexpr auto file_fills = side == BLACK ? NorthFill : SouthFill;

  while (pawns) {
    int square = square::MIndex(pawns);
    Bitboard bb = square::BB(square);
    Bitboard targets = file_fills(bb) & pawns;

    if (targets & pawns) {
      count++;
      bb |= targets;
    }

    pawns ^= bb;
  }

  return count;
}

template <enum Color side>
std::pair<int, int> IsolatedPawns(Bitboard pawns, Bitboard empty_sqs) {
  int open = 0;
  int count = 0;

  while (pawns) {
    int square = square::Index(pawns);
    Bitboard bb = square::BB(square);

    Bitboard targets = FileFill((MOVE_WEST(bb)) | (MOVE_EAST(bb))) & pawns;

    if (!targets) {
      count++;
      open += static_cast<bool>(PushPawn<side>(bb) & empty_sqs);
    }

    pawns ^= bb | targets;
  }

  return std::make_pair(count, open);
}

template <enum Color side>
std::pair<int, int> BackwardPawns(Bitboard side_pawns, Bitboard enemy_pawns) {
  int open = 0;
  int count = 0;
  Bitboard pawns = side_pawns | enemy_pawns;
  Bitboard empty_sqs = ~pawns;
  constexpr auto rank2 = side == WHITE ? kRank2 : kRank7;
  constexpr auto file_fills = side == WHITE ? NorthFill : SouthFill;

  while (side_pawns) {
    int square = square::Index(side_pawns);
    Bitboard bb = square::BB(square);

    Bitboard push_target = PushPawn<side>(bb);
    Bitboard dbl_push_target = PushPawn<side>(push_target);
    Bitboard tpl_push_target = PushPawn<side>(dbl_push_target);
    Bitboard east_targets = PawnTargets<side, EAST>(bb);
    Bitboard west_targets = PawnTargets<side, WEST>(bb);

    Bitboard west_fill_targets = file_fills(MOVE_WEST(push_target));
    Bitboard east_fill_targets = file_fills(MOVE_EAST(push_target));

    Bitboard push_rank = RankMask(square::Index(push_target));
    Bitboard tpl_rank = RankMask(square::Index(tpl_push_target));

    Bitboard ranks_ahead = push_rank | RankMask(square::Index(dbl_push_target));

    // clang-format off
    if (
      (bb & rank2 && ((push_target & pawns) | (dbl_push_target & pawns))) ||
      (bb & rank2 && ((west_fill_targets & (ranks_ahead | tpl_rank) & enemy_pawns) |
        (east_fill_targets & (ranks_ahead | tpl_rank) & enemy_pawns))) ||
      (east_targets & side_pawns && (west_fill_targets & ranks_ahead & enemy_pawns) |
        (push_target & pawns)) ||
      (west_targets & side_pawns && (east_fill_targets & ranks_ahead & enemy_pawns) |
        (push_target & pawns)) ||
      (
        (west_fill_targets & pawns && !(push_rank & pawns)) &&
        (east_fill_targets & pawns && !(push_rank & pawns))
      )
    ) {
      count++;
      open += static_cast<bool>(push_target & empty_sqs);
    }
    // clang-format on

    side_pawns ^= bb;
  }

  return std::make_pair(count, open);
}

template <enum Color side>
int RooksMobility(EvalState &state) {
  int squares = 0;
  Bitboard enemy_sqs;
  Bitboard side_rooks;
  Bitboard empty_sqs = ~state.occupied_sqs;

  if constexpr (side == WHITE) {
    side_rooks = state.white_pieces[ROOK];
    enemy_sqs = square::Occupancy(state.black_pieces);
  } else {
    side_rooks = state.black_pieces[ROOK];
    enemy_sqs = square::Occupancy(state.white_pieces);
  }

  Bitboard movable_sqs = empty_sqs | enemy_sqs;

  BITLOOP(side_rooks) {
    Bitboard moves =
        kSlidingAttacks.Rook(state.occupied_sqs, LOOP_INDEX) & movable_sqs;

    squares += std::popcount(moves) - 7;
  };

  return squares;
}

template <enum Color side>
int BishopsMobility(EvalState &state) {
  int squares = 0;
  Bitboard enemy_sqs;
  Bitboard side_bishops;
  Bitboard empty_sqs = ~state.occupied_sqs;

  if constexpr (side == WHITE) {
    side_bishops = state.white_pieces[BISHOP];
    enemy_sqs = square::Occupancy(state.black_pieces);
  } else {
    side_bishops = state.black_pieces[BISHOP];
    enemy_sqs = square::Occupancy(state.white_pieces);
  }

  Bitboard movable_sqs = empty_sqs | enemy_sqs;

  BITLOOP(side_bishops) {
    Bitboard moves =
        kSlidingAttacks.Bishop(state.occupied_sqs, LOOP_INDEX) & movable_sqs;

    squares += std::popcount(moves) - 6;
  };

  return squares;
}

template <enum Color side>
int KnightsMobility(EvalState &state) {
  int squares = 0;
  Bitboard enemy_sqs;
  Bitboard side_knights;
  Bitboard empty_sqs = ~state.occupied_sqs;

  if constexpr (side == WHITE) {
    side_knights = state.white_pieces[KNIGHT];
    enemy_sqs = square::Occupancy(state.black_pieces);
  } else {
    side_knights = state.black_pieces[KNIGHT];
    enemy_sqs = square::Occupancy(state.white_pieces);
  }

  Bitboard movable_sqs = empty_sqs | enemy_sqs;

  BITLOOP(side_knights) {
    Bitboard moves = kAttackMaps[KNIGHT][LOOP_INDEX] & movable_sqs;

    squares += std::popcount(moves) - 4;
  };

  return squares;
}

template <enum Color side>
int ClosedFiles(EvalState &state) {
  int count = 0;
  Bitboard side_pawns;
  Bitboard side_rooks;

  if constexpr (side == WHITE) {
    side_rooks = state.white_pieces[ROOK];
    side_pawns = state.white_pieces[PAWN];
  } else {
    side_rooks = state.black_pieces[ROOK];
    side_pawns = state.black_pieces[PAWN];
  }

  BITLOOP(side_rooks) {
    Bitboard file = FileMask(LOOP_INDEX);

    if (file & side_pawns) {
      count++;
    }
  }

  return count;
}

template <enum Color side>
std::tuple<int, int, int> SemiOpenFiles(EvalState &state) {
  int count = 0;
  int adj_enemy_king = 0;
  int same_as_enemy_king = 0;

  Bitboard side_rooks;
  Bitboard enemy_pawns;
  Bitboard side_pawns;
  Bitboard enemy_king;
  Bitboard enemy_king_file;

  if constexpr (side == WHITE) {
    side_pawns = state.white_pieces[PAWN];
    side_rooks = state.white_pieces[ROOK];
    enemy_pawns = state.black_pieces[PAWN];

    enemy_king = state.black_pieces[KING];
    enemy_king_file = FileMask(square::Index(enemy_king));
  } else {
    side_pawns = state.black_pieces[PAWN];
    side_rooks = state.black_pieces[ROOK];
    enemy_pawns = state.white_pieces[PAWN];

    enemy_king = state.white_pieces[KING];
    enemy_king_file = FileMask(square::Index(enemy_king));
  }

  BITLOOP(side_rooks) {
    Bitboard file = FileMask(LOOP_INDEX);

    if (!(file & side_pawns) && file & enemy_pawns) {
      count++;

      Bitboard adjacents = ((MOVE_WEST(file)) | (MOVE_EAST(file)));

      same_as_enemy_king += static_cast<bool>(file & enemy_king_file);
      adj_enemy_king += static_cast<bool>(adjacents & enemy_king_file);
    }
  }

  return std::make_tuple(count, adj_enemy_king, same_as_enemy_king);
}

template <enum Color side>
std::tuple<int, int, int> OpenFiles(EvalState &state) {
  int count = 0;
  int adj_enemy_king = 0;
  int same_as_enemy_king = 0;
  Bitboard all_pawns = state.white_pieces[PAWN] | state.black_pieces[PAWN];

  Bitboard side_rooks;
  Bitboard enemy_king;
  Bitboard enemy_king_file;

  if constexpr (side == WHITE) {
    side_rooks = state.white_pieces[ROOK];

    enemy_king = state.black_pieces[KING];
    enemy_king_file = FileMask(square::Index(enemy_king));
  } else {
    side_rooks = state.black_pieces[ROOK];

    enemy_king = state.white_pieces[KING];
    enemy_king_file = FileMask(square::Index(enemy_king));
  }

  BITLOOP(side_rooks) {
    Bitboard file = FileMask(LOOP_INDEX);

    if (!(file & all_pawns)) {
      count++;
      Bitboard adjacents = ((MOVE_WEST(file)) | (MOVE_EAST(file)));

      same_as_enemy_king += static_cast<bool>(file & enemy_king_file);
      adj_enemy_king += static_cast<bool>(adjacents & enemy_king_file);
    }
  }

  return std::make_tuple(count, adj_enemy_king, same_as_enemy_king);
}

template <enum Color side, enum Piece piece>
int Rank7(EvalState &state) {
  if constexpr (side == WHITE) {
    Bitboard hostile_pawns = state.black_pieces[PAWN] & kRank7;
    Bitboard attacking_piece = state.white_pieces[piece] & kRank7;

    return hostile_pawns && std::popcount(attacking_piece);
  }

  Bitboard hostile_pawns = state.white_pieces[PAWN] & kRank2;
  Bitboard attacking_piece = state.black_pieces[piece] & kRank2;

  return hostile_pawns && std::popcount(attacking_piece);
}

template <enum Color side>
int KingDistance(EvalState &state) {
  int score = 0;
  int king_file;

  Bitboard own_queen;
  Bitboard enemy_king;

  if constexpr (side == WHITE) {
    own_queen = state.white_pieces[QUEEN];
    enemy_king = state.black_pieces[KING];
  } else {
    own_queen = state.black_pieces[QUEEN];
    enemy_king = state.white_pieces[KING];
  }

  Coord king_coord;

  CoordForSquare(&king_coord, square::Index(enemy_king));

  king_file = king_coord.file - 97;

  BITLOOP(own_queen) {
    int queen_file;
    Coord queen_coord;

    CoordForSquare(&queen_coord, LOOP_INDEX);

    queen_file = queen_coord.file - 97;

    score += 10 - std::abs(queen_file - king_file) -
             std::abs(queen_coord.rank - king_coord.rank);
  }

  return score;
}

// TODO: implement kings distance & unstoppable passed pawn scoring
template <enum Color side>
std::pair<float, float> PassedPawns(EvalState &state) {
  int side_diff;
  Bitboard side_pawns;
  Bitboard enemy_pawns;
  Bitboard (*front_fill)(Bitboard);

  float opening_score = 0;
  float endgame_score = 0;
  Bitboard empty_sqs = ~state.occupied_sqs;
  Bitboard all_pawns = state.white_pieces[PAWN] | state.black_pieces[PAWN];

  static std::array<float, 8> PP_BONUS{0, 0, 0, 0.1, 0.3, 0.6, 1};

  if constexpr (side == WHITE) {
    side_diff = 0;
    front_fill = NorthFill;
    side_pawns = state.white_pieces[PAWN];
    enemy_pawns = state.black_pieces[PAWN];
  } else {
    side_diff = 8;
    front_fill = SouthFill;
    side_pawns = state.black_pieces[PAWN];
    enemy_pawns = state.white_pieces[PAWN];
  }

  BITLOOP(side_pawns) {
    int square = LOOP_INDEX;
    int rank = square::Rank(square);
    Bitboard bb = square::BB(square);
    Bitboard front_targets = front_fill(bb) ^ bb;

    if (front_targets & all_pawns) {
      continue;
    }

    Bitboard adjacent_sqs = (MOVE_WEST(bb)) | (MOVE_EAST(bb));
    Bitboard targets = front_fill(adjacent_sqs) ^ adjacent_sqs;

    if (targets & enemy_pawns) {
      continue;
    }

    opening_score += 10 + 60 * PP_BONUS[std::abs(rank - side_diff)];

    // endgame evaluation
    int kings_distance = 0;
    int unstoppable_score = 0;

    Bitboard push_target = PushPawn<side>(bb);

    int free_score =
        60 * static_cast<bool>(push_target & empty_sqs &&
                               !(push_target & state.attack_map[OPP(side)]));

    endgame_score +=
        20 + (120 + kings_distance + free_score + unstoppable_score) *
                 PP_BONUS[std::abs(rank - side_diff)];
  }

  return std::make_pair(opening_score, endgame_score);
}

}  // namespace engine
