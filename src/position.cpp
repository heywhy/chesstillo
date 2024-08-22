#include <utility>

#include <chesstillo/board.hpp>
#include <chesstillo/constants.hpp>
#include <chesstillo/fen.hpp>
#include <chesstillo/fill.hpp>
#include <chesstillo/move_gen.hpp>
#include <chesstillo/position.hpp>
#include <chesstillo/types.hpp>

void Position::Reset() {
  board_.Reset();

  king_ban_ = kEmpty;
  castling_rights_ = 0;
  en_passant_sq_ = kEmpty;
}

void Position::Make(Move move) {
  Color opp = OPP(turn_);
  Bitboard &piece = board_.pieces_[turn_][move.piece];
  Bitboard to = BITBOARD_FOR_SQUARE(move.to);
  Bitboard from = BITBOARD_FOR_SQUARE(move.from);

  piece ^= from ^ to;

  if (move.Is(CAPTURE)) {
    Bitboard &piece = board_.pieces_[opp][move.captured];

    piece ^= to;
  }

  if (move.Is(EN_PASSANT)) [[unlikely]] {
    Bitboard &piece = board_.pieces_[opp][PAWN];

    piece ^= move.ep_target;
  }

  if (move.Is(PROMOTION)) [[unlikely]] {
    Bitboard &new_piece = board_.pieces_[turn_][move.promoted];

    // unset the old piece & set the index on the promoted piece
    piece ^= to;
    new_piece ^= to;
  }

  turn_ = opp;

  UpdateInternals();
  UpdateEnPassantSquare(move);

  if (move.piece == PAWN || move.Is(CAPTURE)) {
    halfmove_clock_ = 0;
  } else {
    halfmove_clock_++;
  }

  if (opp == WHITE) {
    fullmove_counter_++;
  }

  moves_.push_front(std::move(move));
}

void Position::Undo(Move &move) {
  Color opp = OPP(turn_);
  Bitboard &piece = board_.pieces_[opp][move.piece];
  Bitboard to = BITBOARD_FOR_SQUARE(move.to);
  Bitboard from = BITBOARD_FOR_SQUARE(move.from);

  piece = (piece ^ to) | from;

  if (move.Is(CAPTURE)) {
    Bitboard *pieces = board_.pieces_[turn_];

    pieces[move.captured] |= to;
  }

  if (move.Is(EN_PASSANT)) [[unlikely]] {
    Bitboard *pieces = board_.pieces_[turn_];

    pieces[PAWN] |= move.ep_target;
  }

  if (opp == BLACK) {
    fullmove_counter_--;
  }

  if (move.Is(CAPTURE) || move.piece == PAWN) {
    halfmove_clock_ = 0;
  }

  turn_ = opp;

  moves_.pop_front();

  UpdateInternals();
  UpdateEnPassantSquare(moves_.front());
}

void Position::UpdateInternals() {
  board_.UpdateOccupiedSqs();

  UpdateKingBan();
}

void Position::UpdateKingBan() {
  king_ban_ = kEmpty;
  Color opp = OPP(turn_);

  Bitboard enemy_pawns = board_.pieces_[opp][PAWN];
  Bitboard enemy_bishop_queen =
      board_.pieces_[opp][BISHOP] | board_.pieces_[opp][QUEEN];
  Bitboard enemy_rook_queen =
      board_.pieces_[opp][ROOK] | board_.pieces_[opp][QUEEN];

  auto [pawn_east_targets, pawn_west_targets] =
      turn_ == WHITE
          ? std::make_tuple(PawnTargets<BLACK, EAST>, PawnTargets<BLACK, WEST>)
          : std::make_tuple(PawnTargets<WHITE, EAST>, PawnTargets<WHITE, WEST>);

  king_ban_ |=
      (KING_ATTACKS(board_.pieces_[opp][KING])) |
      (KNIGHT_ATTACKS(board_.pieces_[opp][KNIGHT])) |
      (BISHOP_ATTACKS(enemy_bishop_queen,
                      ~board_.occupied_sqs_ | board_.pieces_[turn_][KING])) |
      (ROOK_ATTACKS(enemy_rook_queen,
                    ~board_.occupied_sqs_ | board_.pieces_[turn_][KING])) |
      pawn_east_targets(enemy_pawns) | pawn_west_targets(enemy_pawns);
}

Bitboard Position::EnPassantTarget() {
  if (!en_passant_sq_) [[likely]] {
    return kEmpty;
  }

  return turn_ == BLACK ? NorthFill(en_passant_sq_) & kRank4
                        : SouthFill(en_passant_sq_) & kRank5;
}

void Position::UpdateEnPassantSquare(Move &last_move) {
  en_passant_sq_ = kEmpty;

  if (last_move.piece != PAWN) [[likely]] {
    return;
  }

  Bitboard to = BITBOARD_FOR_SQUARE(last_move.to);
  Bitboard from = BITBOARD_FOR_SQUARE(last_move.from);

  Bitboard pawns = board_.pieces_[turn_][PAWN];

  if (!((kRank2 & from && kRank4 & to && kRank4 & pawns) ||
        (kRank7 & from && kRank5 & to && kRank5 & pawns))) [[likely]] {
    return;
  }

  auto [pawn_east_targets, pawn_west_targets, file_fill] =
      turn_ == BLACK ? std::make_tuple(PawnTargets<BLACK, EAST>,
                                       PawnTargets<BLACK, WEST>, SouthFill(to))
                     : std::make_tuple(PawnTargets<WHITE, EAST>,
                                       PawnTargets<WHITE, WEST>, NorthFill(to));

  Bitboard attacked_sqs = pawn_east_targets(pawns) | pawn_west_targets(pawns);

  Bitboard square_behind =
      (file_fill ^ from ^ board_.occupied_sqs_) & file_fill;

  if (square_behind & attacked_sqs) {
    en_passant_sq_ = square_behind;
  }
}
