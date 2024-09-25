#include <bit>
#include <cstdint>
#include <tuple>

#include <chesstillo/board.hpp>
#include <chesstillo/constants.hpp>
#include <chesstillo/evaluation.hpp>
#include <chesstillo/fill.hpp>
#include <chesstillo/move_gen.hpp>
#include <chesstillo/position.hpp>
#include <chesstillo/types.hpp>

void EvalState::ComputeMaterials() {
  for (int i = 0; i < 6; i++) {
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

EvalState EvalState::For(Position &position) {
  Bitboard *white_pieces = position.Pieces(WHITE);
  Bitboard *black_pieces = position.Pieces(BLACK);
  Bitboard occupied_sqs = *position.occupied_sqs_;

  // TODO: maybe move position mask computation into the position
  // class so as to avoid double computation of these masks.
  Bitboard check_mask = CheckMask(position);
  auto [pin_hv_mask, pin_diag_mask] = PinMask(position);

  return {white_pieces, black_pieces, occupied_sqs,
          check_mask,   pin_hv_mask,  pin_diag_mask};
}

// TODO: consider tapered evaluation using game phase alongside other
// properties
float Evaluate(Position &position) {
  EvalState state = EvalState::For(position);
  int side_to_move = position.turn_ == WHITE ? 1 : -1;

  auto [opening_pieces, endgame_pieces] = EvalPieces(state);
  auto [opening_materials, endgame_materials] = EvalMaterials(state);
  auto [opening_pawn_structure, endgame_pawn_structure] =
      EvalPawnStructure(state);

  auto [opening_tempo, endgame_tempo] = std::make_tuple(
      kWeights[TEMPO][0] * side_to_move, kWeights[TEMPO][1] * side_to_move);

  int opening = opening_tempo + opening_materials + opening_pieces -
                opening_pawn_structure;
  int endgame = endgame_tempo + endgame_materials + endgame_pieces -
                endgame_pawn_structure;

  return TAPER_EVAL(opening, endgame, state.phase);
}

std::tuple<int, int> EvalMaterials(EvalState &state) {
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

  return std::make_tuple(scores[0], scores[1]);
}

// TODO: maybe include the "candidate" property from fruits/TOGA
std::tuple<float, float> EvalPawnStructure(EvalState &state) {
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

  return std::make_tuple(scores[0], scores[1]);
}

// TODO: knight outpost, king distance, 7th rank
std::tuple<int, int> EvalPieces(EvalState &state) {
  auto [opening_mobility, endgame_mobility] = EvalMobility(state);
  auto [opening_open_line, endgame_open_line] = EvalOpenFile(state);

  int opening = opening_mobility + opening_open_line;
  int endgame = endgame_mobility + endgame_open_line;

  return std::make_tuple(opening, endgame);
}

std::tuple<int, int> EvalMobility(EvalState &state) {
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

  return std::make_tuple(scores[0], scores[1]);
}

std::tuple<int, int> EvalOpenFile(EvalState &state) {
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

  return std::make_tuple(scores[0], scores[1]);
}

template <enum Color side> int DoublePawns(Bitboard pawns) {
  int count = 0;
  constexpr auto file_fills = side == BLACK ? NorthFill : SouthFill;

  while (pawns) {
    uint8_t square = BIT_INDEX_MSB(pawns);
    Bitboard bb = BITBOARD_FOR_SQUARE(square);
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
std::tuple<int, int> IsolatedPawns(Bitboard pawns, Bitboard empty_sqs) {
  int open = 0;
  int count = 0;

  while (pawns) {
    uint8_t square = BIT_INDEX(pawns);
    Bitboard bb = BITBOARD_FOR_SQUARE(square);

    Bitboard targets = FileFill((MOVE_WEST(bb)) | (MOVE_EAST(bb))) & pawns;

    if (!targets) {
      count++;
      open += static_cast<bool>(PushPawn<side>(bb) & empty_sqs);
    }

    pawns ^= bb | targets;
  }

  return std::make_tuple(count, open);
}

template <enum Color side>
std::tuple<int, int> BackwardPawns(Bitboard side_pawns, Bitboard enemy_pawns) {
  int open = 0;
  int count = 0;
  Bitboard pawns = side_pawns | enemy_pawns;
  Bitboard empty_sqs = ~pawns;
  constexpr auto rank2 = side == WHITE ? kRank2 : kRank7;
  constexpr auto file_fills = side == WHITE ? NorthFill : SouthFill;

  while (side_pawns) {
    uint8_t square = BIT_INDEX(side_pawns);
    Bitboard bb = BITBOARD_FOR_SQUARE(square);

    Bitboard push_target = PushPawn<side>(bb);
    Bitboard dbl_push_target = PushPawn<side>(push_target);
    Bitboard tpl_push_target = PushPawn<side>(dbl_push_target);
    Bitboard east_targets = PawnTargets<side, EAST>(bb);
    Bitboard west_targets = PawnTargets<side, WEST>(bb);

    Bitboard west_fill_targets = file_fills(MOVE_WEST(push_target));
    Bitboard east_fill_targets = file_fills(MOVE_EAST(push_target));

    Bitboard push_rank = RankMask(BIT_INDEX(push_target));
    Bitboard tpl_rank = RankMask(BIT_INDEX(tpl_push_target));

    Bitboard ranks_ahead = push_rank | RankMask(BIT_INDEX(dbl_push_target));

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

  return std::make_tuple(count, open);
}

template <enum Color side> int RooksMobility(EvalState &state) {
  int squares = 0;
  Bitboard enemy_sqs;
  Bitboard side_rooks;
  Bitboard empty_sqs = ~state.occupied_sqs;

  if constexpr (side == WHITE) {
    side_rooks = state.white_pieces[ROOK];
    enemy_sqs = BOARD_OCCUPANCY(state.black_pieces);
  } else {
    side_rooks = state.black_pieces[ROOK];
    enemy_sqs = BOARD_OCCUPANCY(state.white_pieces);
  }

  Bitboard movable_sqs = empty_sqs | enemy_sqs;

  BITLOOP(side_rooks) {
    Bitboard moves =
        kSlidingAttacks.Rook(state.occupied_sqs, LOOP_INDEX) & movable_sqs;

    squares += std::popcount(moves) - 7;
  };

  return squares;
}

template <enum Color side> int BishopsMobility(EvalState &state) {
  int squares = 0;
  Bitboard enemy_sqs;
  Bitboard side_bishops;
  Bitboard empty_sqs = ~state.occupied_sqs;

  if constexpr (side == WHITE) {
    side_bishops = state.white_pieces[BISHOP];
    enemy_sqs = BOARD_OCCUPANCY(state.black_pieces);
  } else {
    side_bishops = state.black_pieces[BISHOP];
    enemy_sqs = BOARD_OCCUPANCY(state.white_pieces);
  }

  Bitboard movable_sqs = empty_sqs | enemy_sqs;

  BITLOOP(side_bishops) {
    Bitboard moves =
        kSlidingAttacks.Bishop(state.occupied_sqs, LOOP_INDEX) & movable_sqs;

    squares += std::popcount(moves) - 6;
  };

  return squares;
}

template <enum Color side> int KnightsMobility(EvalState &state) {
  int squares = 0;
  Bitboard enemy_sqs;
  Bitboard side_knights;
  Bitboard empty_sqs = ~state.occupied_sqs;

  if constexpr (side == WHITE) {
    side_knights = state.white_pieces[KNIGHT];
    enemy_sqs = BOARD_OCCUPANCY(state.black_pieces);
  } else {
    side_knights = state.black_pieces[KNIGHT];
    enemy_sqs = BOARD_OCCUPANCY(state.white_pieces);
  }

  Bitboard movable_sqs = empty_sqs | enemy_sqs;

  BITLOOP(side_knights) {
    Bitboard moves = kAttackMaps[KNIGHT][LOOP_INDEX] & movable_sqs;

    squares += std::popcount(moves) - 4;
  };

  return squares;
}

template <enum Color side> int ClosedFiles(EvalState &state) {
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
    enemy_king_file = FileMask(BIT_INDEX(enemy_king));
  } else {
    side_pawns = state.black_pieces[PAWN];
    side_rooks = state.black_pieces[ROOK];
    enemy_pawns = state.white_pieces[PAWN];

    enemy_king = state.white_pieces[KING];
    enemy_king_file = FileMask(BIT_INDEX(enemy_king));
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
    enemy_king_file = FileMask(BIT_INDEX(enemy_king));
  } else {
    side_rooks = state.black_pieces[ROOK];

    enemy_king = state.white_pieces[KING];
    enemy_king_file = FileMask(BIT_INDEX(enemy_king));
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
