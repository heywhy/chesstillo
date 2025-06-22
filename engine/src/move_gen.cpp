#include <cstdarg>
#include <cstddef>
#include <cstdint>
#include <magic_bits.hpp>
#include <tuple>
#include <utility>

#include <engine/board.hpp>
#include <engine/constants.hpp>
#include <engine/move.hpp>
#include <engine/move_gen.hpp>
#include <engine/position.hpp>
#include <engine/square.hpp>
#include <engine/types.hpp>
#include <engine/utility.hpp>

namespace engine {

inline bool PieceAt(Piece *piece, Piece *mailbox, std::uint_fast8_t square) {
  *piece = mailbox[square];

  return *piece != NONE;
}

MoveList GenerateMoves(Position &position) {
  MoveList move_list;

  move_list.reserve(218);

  Color opp = OPP(position.turn_);
  Bitboard occupied_sqs = position.board_.occupied_sqs;
  PieceList &enemy_pieces = position.Pieces(opp);
  PieceList &own_pieces = position.Pieces(position.turn_);

  Bitboard check_mask = CheckMask(position);
  auto [pin_hv_mask, pin_diag_mask] = PinMask(position);
  Bitboard pin_mask = pin_hv_mask | pin_diag_mask;

  Bitboard empty_sqs = ~occupied_sqs;
  Bitboard own_pieces_bb = square::Occupancy(own_pieces);
  Bitboard enemy_pieces_bb =
      square::Occupancy(enemy_pieces) ^ enemy_pieces[KING];
  Bitboard enemy_or_empty_sqs = enemy_pieces_bb | empty_sqs;
  Bitboard movable_sqs = enemy_or_empty_sqs & check_mask;

  // 1. safe king squares
  std::uint_fast8_t king_sq = square::Index(own_pieces[KING]);
  Bitboard legal_king_moves =
      kAttackMaps[KING][king_sq] & enemy_or_empty_sqs & ~position.king_ban_;

  Bitboard quiet_moves = legal_king_moves & empty_sqs;
  Bitboard captures = legal_king_moves & enemy_pieces_bb;

  BITLOOP(captures) {
    Piece piece;
    std::uint_fast8_t to = LOOP_INDEX;
    Move move(king_sq, to, KING);

    PieceAt(&piece, position.mailbox_, to);

    move.captured = piece;
    move.Set(CAPTURE);

    move_list.push_back(std::move(move));
  }

  BITLOOP(quiet_moves) { move_list.emplace_back(king_sq, LOOP_INDEX, KING); }

  if (check_mask == kEmpty) {
    return move_list;
  }

  // 2. pawn pushes, pawn captures, en passant
  Bitboard side_pawns = own_pieces[PAWN];
  Bitboard pushable_pawns = side_pawns & ~pin_diag_mask;
  Bitboard attackable_pawns = side_pawns & ~pin_hv_mask;

  auto [single_push_targets, double_push_targets, pawn_east_targets,
        pawn_west_targets, file_shift, east_shift, west_shift,
        before_promotion_rank] =
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
    Bitboard pinned_pawns =
        pushable_pawns & pin_hv_mask & ~before_promotion_rank;
    Bitboard free_pawns =
        pushable_pawns & ~pin_hv_mask & ~before_promotion_rank;

    Bitboard free_pawns_pts =
        single_push_targets(free_pawns) & movable_sqs_mask;
    Bitboard pinned_pawns_pts =
        single_push_targets(pinned_pawns) & movable_sqs_mask & pin_hv_mask;

    Bitboard single_targets = free_pawns_pts | pinned_pawns_pts;

    BITLOOP(single_targets) {
      std::uint_fast8_t to = LOOP_INDEX;
      std::uint_fast8_t from = to + file_shift;

      move_list.emplace_back(from, to, PAWN);
    }

    int dbl_file_shift = file_shift * 2;

    Bitboard free_pawns_dts =
        double_push_targets(free_pawns, empty_sqs) & check_mask;
    Bitboard pinned_pawns_dts =
        double_push_targets(pinned_pawns, empty_sqs) & check_mask & pin_hv_mask;

    Bitboard double_targets = free_pawns_dts | pinned_pawns_dts;

    BITLOOP(double_targets) {
      std::uint_fast8_t to = LOOP_INDEX;
      std::uint_fast8_t from = to + dbl_file_shift;

      move_list.emplace_back(from, to, PAWN);
    }
  }

  {
    Bitboard non_promotable = ~before_promotion_rank;
    Bitboard pinned_pawns = attackable_pawns & pin_diag_mask & non_promotable;
    Bitboard free_pawns = attackable_pawns & ~pin_diag_mask & non_promotable;

    Bitboard free_pawns_wts = pawn_west_targets(free_pawns);
    Bitboard pinned_pawns_wts = pawn_west_targets(pinned_pawns) & pin_diag_mask;

    Bitboard free_pawns_ets = pawn_east_targets(free_pawns);
    Bitboard pinned_pawns_ets = pawn_east_targets(pinned_pawns) & pin_diag_mask;

    // 2.2. pawn captures
    {
      Bitboard capture_mask = enemy_pieces_bb & check_mask;
      Bitboard west_targets =
          (free_pawns_wts & capture_mask) | (pinned_pawns_wts & capture_mask);

      BITLOOP(west_targets) {
        Piece piece;
        std::uint_fast8_t to = LOOP_INDEX;
        std::uint_fast8_t from = to + west_shift;
        Move move(from, to, PAWN);

        PieceAt(&piece, position.mailbox_, to);

        move.captured = piece;
        move.Set(CAPTURE);

        move_list.push_back(std::move(move));
      }

      Bitboard east_targets =
          (free_pawns_ets & capture_mask) | (pinned_pawns_ets & capture_mask);

      BITLOOP(east_targets) {
        Piece piece;
        std::uint_fast8_t to = LOOP_INDEX;
        std::uint_fast8_t from = to + east_shift;
        Move move(from, to, PAWN);

        PieceAt(&piece, position.mailbox_, to);

        move.captured = piece;
        move.Set(CAPTURE);

        move_list.push_back(std::move(move));
      }
    }

    // 2.3. en passant
    {
      Bitboard capture_mask =
          IN_CHECK(check_mask) && position.en_passant_target_ & check_mask
              ? position.en_passant_sq_
              : position.en_passant_sq_ & check_mask;

      Bitboard west_targets =
          (free_pawns_wts & capture_mask) | (pinned_pawns_wts & capture_mask);

      BITLOOP(west_targets) {
        std::uint_fast8_t to = LOOP_INDEX;
        std::uint_fast8_t from = to + west_shift;
        Move move(from, to, PAWN);

        move.Set(EN_PASSANT);

        move_list.push_back(std::move(move));
      }

      Bitboard east_targets =
          (free_pawns_ets & capture_mask) | (pinned_pawns_ets & capture_mask);

      BITLOOP(east_targets) {
        std::uint_fast8_t to = LOOP_INDEX;
        int from = to + east_shift;
        Move move(from, to, PAWN);

        move.Set(EN_PASSANT);

        move_list.push_back(std::move(move));
      }
    }
  }

  // 3. remaining legal moves
  {
    Bitboard knights = own_pieces[KNIGHT] & ~pin_mask;

    BITLOOP(knights) {
      std::uint_fast8_t from = LOOP_INDEX;
      Bitboard targets = kAttackMaps[KNIGHT][from] & movable_sqs;

      AddMovesToList(move_list, from, targets, KNIGHT, position.mailbox_,
                     enemy_pieces_bb);
    }
  }

  Bitboard queens = own_pieces[QUEEN];

  {
    Bitboard bishops = own_pieces[BISHOP] & ~pin_hv_mask;
    Bitboard free_bishops = bishops & ~pin_diag_mask;
    Bitboard pinned_bishops = (bishops | queens) & pin_diag_mask;

    BITLOOP(pinned_bishops) {
      std::uint_fast8_t from = LOOP_INDEX;
      Bitboard bb = square::BB(from);
      Bitboard targets = kSlidingAttacks.Bishop(occupied_sqs, from) &
                         movable_sqs & pin_diag_mask;

      targets &= ~own_pieces_bb;

      Piece t = bb & queens ? QUEEN : BISHOP;

      AddMovesToList(move_list, from, targets, t, position.mailbox_,
                     enemy_pieces_bb);
    }

    BITLOOP(free_bishops) {
      std::uint_fast8_t from = LOOP_INDEX;
      Bitboard targets =
          kSlidingAttacks.Bishop(occupied_sqs, from) & movable_sqs;

      targets &= ~own_pieces_bb;

      AddMovesToList(move_list, from, targets, BISHOP, position.mailbox_,
                     enemy_pieces_bb);
    }
  }

  {
    Bitboard rooks = own_pieces[ROOK] & ~pin_diag_mask;
    Bitboard free_rooks = rooks & ~pin_hv_mask;
    Bitboard pinned_rooks = (rooks | queens) & pin_hv_mask;

    BITLOOP(pinned_rooks) {
      std::uint_fast8_t from = LOOP_INDEX;
      Bitboard bb = square::BB(from);
      Bitboard targets =
          kSlidingAttacks.Rook(occupied_sqs, from) & movable_sqs & pin_hv_mask;

      targets &= ~own_pieces_bb;

      Piece t = bb & queens ? QUEEN : ROOK;

      AddMovesToList(move_list, from, targets, t, position.mailbox_,
                     enemy_pieces_bb);
    }

    BITLOOP(free_rooks) {
      std::uint_fast8_t from = LOOP_INDEX;

      Bitboard targets = kSlidingAttacks.Rook(occupied_sqs, from) & movable_sqs;

      targets &= ~own_pieces_bb;

      AddMovesToList(move_list, from, targets, ROOK, position.mailbox_,
                     enemy_pieces_bb);
    }
  }

  {
    Bitboard mqueens = own_pieces[QUEEN] & ~pin_mask;

    BITLOOP(mqueens) {
      std::uint_fast8_t from = LOOP_INDEX;

      Bitboard targets =
          kSlidingAttacks.Queen(occupied_sqs, from) & movable_sqs;

      targets &= ~own_pieces_bb;

      AddMovesToList(move_list, from, targets, QUEEN, position.mailbox_,
                     enemy_pieces_bb);
    }
  }

  // 5. castling
  {
    auto [left_rook, right_rook, starting_rank] =
        position.turn_ == WHITE ? std::make_tuple(a1, h1, kRank1)
                                : std::make_tuple(a8, h8, kRank8);

    std::uint_fast8_t king_side = CASTLE_RIGHT(position.turn_);
    std::uint_fast8_t queen_side = CASTLE_LEFT(position.turn_);
    Bitboard rooks = own_pieces[ROOK];
    Bitboard king = own_pieces[KING] & starting_rank;
    Bitboard king_side_rook = rooks & square::BB(right_rook);
    Bitboard queen_side_rook = rooks & square::BB(left_rook);
    Bitboard king_side_path = kCastleKingSidePath & starting_rank;
    Bitboard queen_side_path = kCastleQueenSidePath & starting_rank;
    Bitboard right_occupied =
        (occupied_sqs ^ king_side_rook ^ king) & kKingSide & starting_rank;
    Bitboard left_occupied =
        (occupied_sqs ^ queen_side_rook) & kQueenSide & starting_rank;

    if (check_mask == kUniverse && position.castling_rights_ & king_side &&
        king_side_rook && right_occupied == kEmpty &&
        !(king_side_path & position.king_ban_)) {
      Move move(square::Index(king), square::Index(king << 2), KING);

      move.Set(CASTLE_RIGHT);

      move_list.push_back(std::move(move));
    }

    if (check_mask == kUniverse && position.castling_rights_ & queen_side &&
        queen_side_rook && left_occupied == kEmpty &&
        !(queen_side_path & position.king_ban_)) {
      Move move(square::Index(king), square::Index(king >> 2), KING);

      move.Set(CASTLE_LEFT);

      move_list.push_back(std::move(move));
    }
  }

  // 6. pawn promotions
  {
    Bitboard movable_sqs_mask = empty_sqs & check_mask;
    Bitboard pinned_pawns =
        pushable_pawns & pin_hv_mask & before_promotion_rank;
    Bitboard free_pawns = pushable_pawns & ~pin_hv_mask & before_promotion_rank;

    Bitboard free_pawns_pts =
        single_push_targets(free_pawns) & movable_sqs_mask;
    Bitboard pinned_pawns_pts =
        single_push_targets(pinned_pawns) & movable_sqs_mask & pin_hv_mask;

    Bitboard single_targets = free_pawns_pts | pinned_pawns_pts;

    BITLOOP(single_targets) {
      std::uint_fast8_t to = LOOP_INDEX;
      std::uint_fast8_t from = LOOP_INDEX + file_shift;

      for (Piece piece : {QUEEN, ROOK, BISHOP, KNIGHT}) {
        Move move(from, to, PAWN);

        move.Set(PROMOTION);
        move.promoted = piece;

        move_list.push_back(std::move(move));
      }
    }

    {
      Bitboard capture_mask = enemy_pieces_bb & check_mask;
      Bitboard pinned_pawns =
          attackable_pawns & pin_diag_mask & before_promotion_rank;
      Bitboard free_pawns =
          attackable_pawns & ~pin_diag_mask & before_promotion_rank;

      Bitboard free_pawns_wts = pawn_west_targets(free_pawns) & capture_mask;
      Bitboard pinned_pawns_wts =
          pawn_west_targets(pinned_pawns) & capture_mask & pin_diag_mask;
      Bitboard west_targets = free_pawns_wts | pinned_pawns_wts;

      BITLOOP(west_targets) {
        Piece enemy_piece;
        std::uint_fast8_t to = LOOP_INDEX;
        std::uint_fast8_t from = to + west_shift;

        PieceAt(&enemy_piece, position.mailbox_, to);

        for (Piece piece : {QUEEN, ROOK, BISHOP, KNIGHT}) {
          Move move(from, to, PAWN);

          move.Set(CAPTURE);
          move.Set(PROMOTION);

          move.promoted = piece;
          move.captured = enemy_piece;

          move_list.push_back(std::move(move));
        }
      }

      Bitboard free_pawns_ets = pawn_east_targets(free_pawns) & capture_mask;
      Bitboard pinned_pawns_ets =
          pawn_east_targets(pinned_pawns) & capture_mask & pin_diag_mask;
      Bitboard east_targets = free_pawns_ets | pinned_pawns_ets;

      BITLOOP(east_targets) {
        Piece enemy_piece;
        std::uint_fast8_t to = LOOP_INDEX;
        std::uint_fast8_t from = to + east_shift;

        PieceAt(&enemy_piece, position.mailbox_, to);

        for (Piece piece : {QUEEN, ROOK, BISHOP, KNIGHT}) {
          Move move(from, to, PAWN);

          move.Set(CAPTURE);
          move.Set(PROMOTION);

          move.promoted = piece;
          move.captured = enemy_piece;

          move_list.push_back(std::move(move));
        }
      }
    }
  }

  return move_list;
}

void AddMovesToList(MoveList &move_list, std::uint_fast8_t from,
                    Bitboard targets, Piece piece, Piece *mailbox,
                    Bitboard enemy_bb) {
  BITLOOP(targets) {
    Piece captured;
    Move move(from, LOOP_INDEX, piece);
    Bitboard bb = square::BB(LOOP_INDEX);

    if (bb & enemy_bb && PieceAt(&captured, mailbox, LOOP_INDEX)) {
      move.Set(CAPTURE);
      move.captured = captured;
    }

    move_list.push_back(std::move(move));
  }
}

Bitboard CheckMask(Position &position) {
  Bitboard mask = kUniverse;
  Color opp = OPP(position.turn_);
  Bitboard occupied_sqs = position.board_.occupied_sqs;
  PieceList &opp_pieces = position.Pieces(opp);
  PieceList &own_pieces = position.Pieces(position.turn_);
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

  std::uint_fast8_t king_square = square::Index(king_bb);

  Bitboard knight = kAttackMaps[KNIGHT][king_square] & opp_pieces[KNIGHT];

  if (knight) {
    mask = knight;
  }

  Bitboard enemy_bishop_queen = opp_pieces[BISHOP] | opp_pieces[QUEEN];

  if (kAttackMaps[BISHOP][king_square] & enemy_bishop_queen) {
    Bitboard pieces =
        kSlidingAttacks.Bishop(occupied_sqs, king_square) & enemy_bishop_queen;

    BITLOOP(pieces) {
      std::uint_fast8_t bishop = LOOP_INDEX;
      Bitboard rays_from_attacker = kCheckBetween[BISHOP][king_square][bishop];

      if (mask != kUniverse) {
        return kEmpty;
      }

      mask = rays_from_attacker | square::BB(bishop);
    }
  }

  Bitboard enemy_rook_queen = opp_pieces[ROOK] | opp_pieces[QUEEN];

  if (kAttackMaps[ROOK][king_square] & enemy_rook_queen) {
    Bitboard pieces =
        kSlidingAttacks.Rook(occupied_sqs, king_square) & enemy_rook_queen;

    BITLOOP(pieces) {
      std::uint_fast8_t rook = LOOP_INDEX;
      Bitboard rays_from_attacker = kCheckBetween[ROOK][king_square][rook];

      if (mask != kUniverse) {
        return kEmpty;
      }

      mask = rays_from_attacker | square::BB(rook);
    }
  }

  return mask;
}

std::pair<Bitboard, Bitboard> PinMask(Position &position) {
  Color opp = OPP(position.turn_);
  PieceList &own_pieces = position.Pieces(position.turn_);
  PieceList &opp_pieces = position.Pieces(opp);
  Bitboard occupied_sqs = position.board_.occupied_sqs;
  Bitboard king_bb = own_pieces[KING];
  Bitboard own_pieces_bb = square::Occupancy(own_pieces);

  Bitboard hv_mask = kEmpty;
  Bitboard diag_mask = kEmpty;

  Bitboard enemy_rook_queen = opp_pieces[ROOK] | opp_pieces[QUEEN];
  Bitboard enemy_bishop_queen = opp_pieces[BISHOP] | opp_pieces[QUEEN];

  std::uint_fast8_t king_square = square::Index(king_bb);

  if (kAttackMaps[ROOK][king_square] & enemy_rook_queen) {
    Bitboard attacks_from_king =
        kSlidingAttacks.Rook(occupied_sqs, king_square);
    Bitboard xray_attacks_from_king = RookXRayAttacks(
        attacks_from_king, occupied_sqs, own_pieces_bb, king_square);

    Bitboard pieces = xray_attacks_from_king & enemy_rook_queen;

    BITLOOP(pieces) {
      std::uint_fast8_t rook = LOOP_INDEX;
      Bitboard rays_from_attacker = kCheckBetween[ROOK][king_square][rook];

      hv_mask |= rays_from_attacker | square::BB(rook);
    }
  }

  if (kAttackMaps[BISHOP][king_square] & enemy_bishop_queen) {
    Bitboard attacks_from_king =
        kSlidingAttacks.Bishop(occupied_sqs, king_square);
    Bitboard xray_attacks_from_king = BishopXRayAttacks(
        attacks_from_king, occupied_sqs, own_pieces_bb, king_square);

    Bitboard pieces = xray_attacks_from_king & enemy_bishop_queen;

    BITLOOP(pieces) {
      std::uint_fast8_t bishop = LOOP_INDEX;
      Bitboard rays_from_attacker = kCheckBetween[BISHOP][king_square][bishop];

      diag_mask |= rays_from_attacker | square::BB(bishop);
    }
  }

  return {hv_mask, diag_mask};
}

}  // namespace engine
