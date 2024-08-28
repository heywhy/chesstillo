#include <cstdarg>
#include <cstddef>
#include <cstdint>
#include <tuple>
#include <utility>
#include <vector>

#include <magic_bits.hpp>

#include <chesstillo/board.hpp>
#include <chesstillo/constants.hpp>
#include <chesstillo/move_gen.hpp>
#include <chesstillo/position.hpp>
#include <chesstillo/types.hpp>
#include <chesstillo/utility.hpp>

// The max number of squares any piece can travel to is that of the queen
#define MAX_PIECE_MOVES 27
// Assuming the maximum number of any piece type asides king &
// pawns is 10
#define MAX_PIECE_NUMBER 10
#define MAX_MOVES_BUFFER_SIZE 256

bool PieceAt(Piece *piece, Bitboard *pieces, unsigned int square) {
  Bitboard bb = BITBOARD_FOR_SQUARE(square);

  if (pieces[PAWN] & bb) {
    *piece = PAWN;
    return true;
  }

  if (pieces[ROOK] & bb) {
    *piece = ROOK;
    return true;
  }

  if (pieces[KNIGHT] & bb) {
    *piece = KNIGHT;
    return true;
  }

  if (pieces[BISHOP] & bb) {
    *piece = BISHOP;
    return true;
  }

  if (pieces[QUEEN] & bb) {
    *piece = QUEEN;
    return true;
  }

  if (pieces[KING] & bb) {
    *piece = KING;
    return true;
  }

  return false;
}

std::vector<Move> GenerateMoves(Position &position) {
  std::vector<Move> moves;

  moves.reserve(218);

  Color opp = OPP(position.turn_);
  Bitboard occupied_sqs = *position.occupied_sqs_;
  Bitboard *enemy_pieces = position.Pieces(opp);
  Bitboard *own_pieces = position.Pieces(position.turn_);

  Bitboard check_mask = CheckMask(position);
  auto [pin_hv_mask, pin_diag_mask] = PinMask(position);
  Bitboard pin_mask = pin_hv_mask | pin_diag_mask;

  Bitboard empty_sqs = ~occupied_sqs;
  Bitboard own_pieces_bb = BOARD_OCCUPANCY(own_pieces);
  Bitboard enemy_pieces_bb = BOARD_OCCUPANCY(enemy_pieces) ^ enemy_pieces[KING];
  Bitboard enemy_or_empty_sqs = enemy_pieces_bb | empty_sqs;
  Bitboard movable_sqs = enemy_or_empty_sqs & check_mask;

  // 1. safe king squares
  int king_sq = BIT_INDEX(own_pieces[KING]);
  Bitboard legal_king_moves =
      kAttackMaps[KING][king_sq] & enemy_or_empty_sqs & ~position.king_ban_;

  Bitboard quiet_moves = legal_king_moves & empty_sqs;
  Bitboard captures = legal_king_moves & enemy_pieces_bb;

  BITLOOP(captures) {
    Piece piece;
    Move move(king_sq, LOOP_INDEX, KING);

    PieceAt(&piece, enemy_pieces, LOOP_INDEX);

    move.captured = piece;
    move.Set(CAPTURE);

    moves.push_back(std::move(move));
  }

  BITLOOP(quiet_moves) { moves.emplace_back(king_sq, LOOP_INDEX, KING); }

  if (check_mask == kEmpty) {
    return moves;
  }

  // 2. pawn pushes, pawn captures
  Bitboard side_pawns = own_pieces[PAWN];
  Bitboard pushable_pawns = side_pawns & ~pin_diag_mask;
  Bitboard attackable_pawns = side_pawns & ~pin_hv_mask;

  auto [single_push_targets, double_push_targets, pawn_east_targets,
        pawn_west_targets, file_shift, east_shift, west_shift,
        pre_promotion_rank] =
      position.turn_ == WHITE
          ? std::make_tuple(PushPawn<WHITE>, DoublePushPawn<WHITE>,
                            PawnTargets<WHITE, EAST>, PawnTargets<WHITE, WEST>,
                            -8, -9, -7, kRank7)
          : std::make_tuple(PushPawn<BLACK>, DoublePushPawn<BLACK>,
                            PawnTargets<BLACK, EAST>, PawnTargets<BLACK, WEST>,
                            8, 7, 9, kRank2);

  // 2.1. pawn pushes
  {
    Bitboard movable_sqs_mask = empty_sqs & check_mask;
    Bitboard pinned_pawns = pushable_pawns & pin_hv_mask;
    Bitboard free_pawns = pushable_pawns & ~pin_hv_mask;

    Bitboard free_pawns_pts =
        single_push_targets(free_pawns) & movable_sqs_mask;
    Bitboard pinned_pawns_pts =
        single_push_targets(pinned_pawns) & movable_sqs_mask & pin_hv_mask;

    Bitboard single_targets = free_pawns_pts | pinned_pawns_pts;

    BITLOOP(single_targets) {
      int from = LOOP_INDEX + file_shift;

      moves.emplace_back(from, LOOP_INDEX, PAWN);
    }

    file_shift *= 2;

    Bitboard free_pawns_dts =
        double_push_targets(free_pawns, empty_sqs) & check_mask;
    Bitboard pinned_pawns_dts =
        double_push_targets(pinned_pawns, empty_sqs) & check_mask & pin_hv_mask;

    Bitboard double_targets = free_pawns_dts | pinned_pawns_dts;

    BITLOOP(double_targets) {
      moves.emplace_back(LOOP_INDEX + file_shift, LOOP_INDEX, PAWN);
    }
  }

  // 2.2. pawn captures
  {
    Bitboard capture_mask = enemy_pieces_bb & check_mask;
    Bitboard pinned_pawns = attackable_pawns & pin_diag_mask;
    Bitboard free_pawns = attackable_pawns & ~pin_diag_mask;

    Bitboard free_pawns_wts = pawn_west_targets(free_pawns) & capture_mask;
    Bitboard pinned_pawns_wts =
        pawn_west_targets(pinned_pawns) & capture_mask & pin_diag_mask;
    Bitboard west_targets = free_pawns_wts | pinned_pawns_wts;

    BITLOOP(west_targets) {
      Piece piece;
      int from = LOOP_INDEX + west_shift;
      Move move(from, LOOP_INDEX, PAWN);

      PieceAt(&piece, enemy_pieces, LOOP_INDEX);

      move.captured = piece;
      move.Set(CAPTURE);

      moves.push_back(std::move(move));
    }

    Bitboard free_pawns_ets = pawn_east_targets(free_pawns) & capture_mask;
    Bitboard pinned_pawns_ets =
        pawn_east_targets(pinned_pawns) & capture_mask & pin_diag_mask;
    Bitboard east_targets = free_pawns_ets | pinned_pawns_ets;

    BITLOOP(east_targets) {
      Piece piece;
      int from = LOOP_INDEX + east_shift;
      Move move(from, LOOP_INDEX, PAWN);

      PieceAt(&piece, enemy_pieces, LOOP_INDEX);

      move.captured = piece;
      move.Set(CAPTURE);

      moves.push_back(std::move(move));
    }
  }

  // 3. remaining legal moves
  {
    Bitboard knights = own_pieces[KNIGHT] & ~pin_mask;

    BITLOOP(knights) {
      Bitboard targets = kAttackMaps[KNIGHT][LOOP_INDEX] & movable_sqs;

      AddMovesToList(moves, LOOP_INDEX, targets, KNIGHT, enemy_pieces,
                     enemy_pieces_bb);
    }
  }

  Bitboard queens = own_pieces[QUEEN];

  {
    Bitboard bishops = own_pieces[BISHOP] & ~pin_hv_mask;
    Bitboard free_bishops = bishops & ~pin_diag_mask;
    Bitboard pinned_bishops = (bishops | queens) & pin_diag_mask;

    BITLOOP(pinned_bishops) {
      Bitboard bb = BITBOARD_FOR_SQUARE(LOOP_INDEX);
      Bitboard targets = kSlidingAttacks.Bishop(occupied_sqs, LOOP_INDEX) &
                         movable_sqs & pin_diag_mask;

      targets &= ~own_pieces_bb;

      Piece t = bb & queens ? QUEEN : BISHOP;

      AddMovesToList(moves, LOOP_INDEX, targets, t, enemy_pieces,
                     enemy_pieces_bb);
    }

    BITLOOP(free_bishops) {
      Bitboard targets =
          kSlidingAttacks.Bishop(occupied_sqs, LOOP_INDEX) & movable_sqs;

      targets &= ~own_pieces_bb;

      AddMovesToList(moves, LOOP_INDEX, targets, BISHOP, enemy_pieces,
                     enemy_pieces_bb);
    }
  }

  {
    Bitboard rooks = own_pieces[ROOK] & ~pin_diag_mask;
    Bitboard free_rooks = rooks & ~pin_hv_mask;
    Bitboard pinned_rooks = (rooks | queens) & pin_hv_mask;

    BITLOOP(pinned_rooks) {
      Bitboard bb = BITBOARD_FOR_SQUARE(LOOP_INDEX);
      Bitboard targets = kSlidingAttacks.Rook(occupied_sqs, LOOP_INDEX) &
                         movable_sqs & pin_hv_mask;

      targets &= ~own_pieces_bb;

      Piece t = bb & queens ? QUEEN : ROOK;

      AddMovesToList(moves, LOOP_INDEX, targets, t, enemy_pieces,
                     enemy_pieces_bb);
    }

    BITLOOP(free_rooks) {
      Bitboard targets =
          kSlidingAttacks.Rook(occupied_sqs, LOOP_INDEX) & movable_sqs;

      targets &= ~own_pieces_bb;

      AddMovesToList(moves, LOOP_INDEX, targets, ROOK, enemy_pieces,
                     enemy_pieces_bb);
    }
  }

  {
    Bitboard mqueens = own_pieces[QUEEN] & ~pin_mask;

    BITLOOP(mqueens) {
      Bitboard targets =
          kSlidingAttacks.Queen(occupied_sqs, LOOP_INDEX) & movable_sqs;

      targets &= ~own_pieces_bb;

      AddMovesToList(moves, LOOP_INDEX, targets, QUEEN, enemy_pieces,
                     enemy_pieces_bb);
    }
  }

  // 4 en passant, pawn promotions
  // 4.1. en passant
  {
    Bitboard west_targets = pawn_west_targets(attackable_pawns) &
                            position.en_passant_sq_ & check_mask;

    BITLOOP(west_targets) {
      int from = LOOP_INDEX + west_shift;

      Move move(from, LOOP_INDEX, PAWN);

      move.Set(EN_PASSANT);

      moves.push_back(std::move(move));
    }

    Bitboard east_targets = pawn_east_targets(attackable_pawns) &
                            position.en_passant_sq_ & check_mask;

    BITLOOP(east_targets) {
      int from = LOOP_INDEX + east_shift;

      Move move(from, LOOP_INDEX, PAWN);

      move.Set(EN_PASSANT);

      moves.push_back(std::move(move));
    }
  }

  // 5. castling
  {
    auto [left_rook, right_rook, starting_rank] =
        position.turn_ == WHITE ? std::make_tuple(a1, h1, kRank1)
                                : std::make_tuple(a8, h8, kRank8);

    std::uint8_t king_side = CASTLE_RIGHT(position.turn_);
    std::uint8_t queen_side = CASTLE_LEFT(position.turn_);
    Bitboard rooks = own_pieces[ROOK];
    Bitboard king = own_pieces[KING] & starting_rank;
    Bitboard king_side_rook = rooks & BITBOARD_FOR_SQUARE(right_rook);
    Bitboard queen_side_rook = rooks & BITBOARD_FOR_SQUARE(left_rook);
    Bitboard king_side_path = kCastleKingSidePath & starting_rank;
    Bitboard queen_side_path = kCastleQueenSidePath & starting_rank;
    Bitboard right_occupied =
        (occupied_sqs ^ king_side_rook ^ king) & kKingSide & starting_rank;
    Bitboard left_occupied =
        (occupied_sqs ^ queen_side_rook) & kQueenSide & starting_rank;

    if (check_mask == kUniverse && position.castling_rights_ & king_side &&
        king_side_rook && !right_occupied &&
        !(king_side_path & position.king_ban_)) {
      Move move(BIT_INDEX(king), BIT_INDEX(king << 2), KING);

      move.Set(CASTLE_RIGHT);

      moves.push_back(std::move(move));
    }

    if (check_mask == kUniverse && position.castling_rights_ & queen_side &&
        queen_side_rook && !left_occupied &&
        !(queen_side_path & position.king_ban_)) {
      Move move(BIT_INDEX(king), BIT_INDEX(king >> 2), KING);

      move.Set(CASTLE_LEFT);

      moves.push_back(std::move(move));
    }
  }

  // 2.4. pawn promotions
  // Bitboard promotable_pawns = movable_pawns & pre_promotion_rank;
  // Bitboard non_promotable_pawns = movable_pawns & ~pre_promotion_rank;
  // Bitboard push_promotion_targets =
  //     single_push_targets(promotable_pawns) & empty_sqs & check_mask;
  //
  // BITLOOP(push_promotion_targets) {
  //   int from = LOOP_INDEX + file_shift;
  //
  //   for (Piece piece : {QUEEN, ROOK, BISHOP, KNIGHT}) {
  //     Move move(from, LOOP_INDEX, PAWN);
  //
  //     move.Set(PROMOTION);
  //     move.promoted = piece;
  //
  //     moves.push_back(std::move(move));
  //   }
  // }
  //
  // Bitboard east_promotion_targets =
  //     pawn_east_targets(promotable_pawns) & enemy_pieces_bb;
  //
  // BITLOOP(east_promotion_targets) {
  //   Piece enemy_piece;
  //   Bitboard from = LOOP_INDEX + east_shift;
  //   Move move(from, LOOP_INDEX, PAWN);
  //
  //   PieceAt(&enemy_piece, enemy_pieces, LOOP_INDEX);
  //
  //   for (Piece piece : {QUEEN, ROOK, BISHOP, KNIGHT}) {
  //     Move move(from, LOOP_INDEX, PAWN);
  //
  //     move.Set(CAPTURE);
  //     move.Set(PROMOTION);
  //
  //     move.promoted = piece;
  //     move.captured = enemy_piece;
  //
  //     moves.push_back(std::move(move));
  //   }
  // }
  //
  // Bitboard west_promotion_targets =
  //     pawn_west_targets(promotable_pawns) & enemy_pieces_bb;
  //
  // BITLOOP(west_promotion_targets) {
  //   Piece enemy_piece;
  //   Bitboard from = LOOP_INDEX + west_shift;
  //   Move move(from, LOOP_INDEX, PAWN);
  //
  //   PieceAt(&enemy_piece, enemy_pieces, LOOP_INDEX);
  //
  //   for (Piece piece : {QUEEN, ROOK, BISHOP, KNIGHT}) {
  //     Move move(from, LOOP_INDEX, PAWN);
  //
  //     move.Set(CAPTURE);
  //     move.Set(PROMOTION);
  //
  //     move.promoted = piece;
  //     move.captured = enemy_piece;
  //
  //     moves.push_back(std::move(move));
  //   }
  // }

  return moves;
}

void AddMovesToList(std::vector<Move> &moves, int from, Bitboard targets,
                    Piece piece, Bitboard *enemy_pieces, Bitboard enemy_bb) {
  BITLOOP(targets) {
    Piece captured;
    Move move(from, LOOP_INDEX, piece);
    Bitboard bb = BITBOARD_FOR_SQUARE(LOOP_INDEX);

    if (bb & enemy_bb && PieceAt(&captured, enemy_pieces, LOOP_INDEX)) {
      move.Set(CAPTURE);
      move.captured = captured;
    }

    moves.push_back(std::move(move));
  }
}

Bitboard CheckMask(Position &position) {
  Bitboard mask = kUniverse;
  Color opp = OPP(position.turn_);
  Bitboard occupied_sqs = *position.occupied_sqs_;
  Bitboard *opp_pieces = position.Pieces(opp);
  Bitboard *own_pieces = position.Pieces(position.turn_);
  Bitboard king_bb = own_pieces[KING];

  auto [pawn_east_targets, pawn_west_targets, inverse_east_targets,
        inverse_west_targets] =
      position.turn_ == BLACK
          ? std::make_tuple(PawnTargets<WHITE, EAST>, PawnTargets<WHITE, WEST>,
                            PawnTargets<BLACK, WEST>, PawnTargets<BLACK, EAST>)
          : std::make_tuple(PawnTargets<BLACK, EAST>, PawnTargets<BLACK, WEST>,
                            PawnTargets<WHITE, WEST>, PawnTargets<WHITE, EAST>);

  if (pawn_east_targets(opp_pieces[PAWN]) & king_bb) {
    mask = inverse_east_targets(king_bb);
  } else if (pawn_west_targets(opp_pieces[PAWN]) & king_bb) {
    mask = inverse_west_targets(king_bb);
  }

  int king_square = BIT_INDEX(king_bb);

  Bitboard knight = kAttackMaps[KNIGHT][king_square] & opp_pieces[KNIGHT];

  if (knight) {
    mask = knight;
  }

  Bitboard enemy_bishop_queen = opp_pieces[BISHOP] | opp_pieces[QUEEN];

  if (kAttackMaps[BISHOP][king_square] & enemy_bishop_queen) {
    Bitboard pieces =
        kSlidingAttacks.Bishop(occupied_sqs, king_square) & enemy_bishop_queen;

    if (mask != kUniverse && pieces) {
      return kEmpty;
    }

    BITLOOP(pieces) {
      int bishop = LOOP_INDEX;
      Bitboard rays_from_attacker = kCheckBetween[BISHOP][king_square][bishop];

      mask = rays_from_attacker | BITBOARD_FOR_SQUARE(bishop);
    }
  }

  Bitboard enemy_rook_queen = opp_pieces[ROOK] | opp_pieces[QUEEN];

  if (kAttackMaps[ROOK][king_square] & enemy_rook_queen) {
    Bitboard pieces =
        kSlidingAttacks.Rook(occupied_sqs, king_square) & enemy_rook_queen;

    if (mask != kUniverse && pieces) {
      return kEmpty;
    }

    BITLOOP(pieces) {
      int rook = LOOP_INDEX;
      Bitboard rays_from_attacker = kCheckBetween[ROOK][king_square][rook];

      mask = rays_from_attacker | BITBOARD_FOR_SQUARE(rook);
    }
  }

  return mask;
}

std::tuple<Bitboard, Bitboard> PinMask(Position &position) {
  Color opp = OPP(position.turn_);
  Bitboard *own_pieces = position.Pieces(position.turn_);
  Bitboard *opp_pieces = position.Pieces(opp);
  Bitboard occupied_sqs = *position.occupied_sqs_;
  Bitboard king_bb = own_pieces[KING];
  Bitboard own_pieces_bb = BOARD_OCCUPANCY(own_pieces);

  Bitboard hv_mask = kEmpty;
  Bitboard diag_mask = kEmpty;

  Bitboard enemy_rook_queen = opp_pieces[ROOK] | opp_pieces[QUEEN];
  Bitboard enemy_bishop_queen = opp_pieces[BISHOP] | opp_pieces[QUEEN];

  int king_square = BIT_INDEX(king_bb);

  if (kAttackMaps[ROOK][king_square] & enemy_rook_queen) {
    Bitboard attacks_from_king =
        kSlidingAttacks.Rook(occupied_sqs, king_square);
    Bitboard xray_attacks_from_king = RookXRayAttacks(
        attacks_from_king, occupied_sqs, own_pieces_bb, king_square);

    Bitboard pieces = xray_attacks_from_king & enemy_rook_queen;

    BITLOOP(pieces) {
      int rook = LOOP_INDEX;
      Bitboard rays_from_attacker = kCheckBetween[ROOK][king_square][rook];

      hv_mask |= rays_from_attacker | BITBOARD_FOR_SQUARE(rook);
    }
  }

  if (kAttackMaps[BISHOP][king_square] & enemy_bishop_queen) {
    Bitboard attacks_from_king =
        kSlidingAttacks.Bishop(occupied_sqs, king_square);
    Bitboard xray_attacks_from_king = BishopXRayAttacks(
        attacks_from_king, occupied_sqs, own_pieces_bb, king_square);

    Bitboard pieces = xray_attacks_from_king & enemy_bishop_queen;

    BITLOOP(pieces) {
      int bishop = LOOP_INDEX;
      Bitboard rays_from_attacker = kCheckBetween[BISHOP][king_square][bishop];

      diag_mask |= rays_from_attacker | BITBOARD_FOR_SQUARE(bishop);
    }
  }

  return {hv_mask, diag_mask};
}
