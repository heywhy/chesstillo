#include <tuple>
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
  history_ = kEmptyStack;
}

void Position::Make(Move move) {
  history_.emplace(en_passant_sq_, en_passant_sq_, castling_rights_,
                   halfmove_clock_);

  Color opp = OPP(turn_);
  Bitboard &piece = board_.pieces_[turn_][move.piece];
  Bitboard to = BITBOARD_FOR_SQUARE(move.to);
  Bitboard from = BITBOARD_FOR_SQUARE(move.from);

  piece ^= from ^ to;

  if (move.Is(CAPTURE)) {
    Bitboard &piece = board_.pieces_[opp][move.captured];

    piece ^= to;
  }

  if (move.piece == KING && move.Is(CASTLE_RIGHT)) [[unlikely]] {
    int king_square = BIT_INDEX(piece);
    Bitboard rank = RankMask(king_square);
    Bitboard &rooks = board_.pieces_[turn_][ROOK];
    Bitboard rook = rooks & rank & kKingSide;

    rooks ^= rook | (rook >> 2);
  }

  if (move.piece == KING && move.Is(CASTLE_LEFT)) [[unlikely]] {
    int king_square = BIT_INDEX(piece);
    Bitboard rank = RankMask(king_square);
    Bitboard &rooks = board_.pieces_[turn_][ROOK];
    Bitboard rook = rooks & rank & kQueenSide;

    rooks ^= rook | (rook << 3);
  }

  {
    Bitboard castle_left = CASTLE_LEFT(turn_);
    Bitboard castle_right = CASTLE_RIGHT(turn_);

    auto [left_rook, right_rook] =
        turn_ == WHITE ? std::make_tuple(a1, h1) : std::make_tuple(a8, h8);

    if ((move.piece == KING ||
         (move.piece == ROOK && move.from == left_rook)) &&
        castling_rights_ & castle_left) [[unlikely]] {
      castling_rights_ ^= castle_left;
    }

    if ((move.piece == KING ||
         (move.piece == ROOK && move.from == right_rook)) &&
        castling_rights_ & castle_right) [[unlikely]] {
      castling_rights_ ^= castle_right;
    }
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

  if (move.piece == PAWN || move.Is(CAPTURE)) {
    halfmove_clock_ = 0;
  } else {
    halfmove_clock_++;
  }

  if (opp == WHITE) {
    fullmove_counter_++;
  }

  moves_.push_front(std::move(move));

  UpdateInternals();
  UpdateEnPassantSquare(move);
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

  if (move.piece == KING && move.Is(CASTLE_RIGHT)) [[unlikely]] {
    int king_square = BIT_INDEX(piece);
    Bitboard rank = RankMask(king_square);
    Bitboard &rooks = board_.pieces_[opp][ROOK];
    Bitboard rook = rooks & rank & kKingSide;

    rooks ^= rook | (rook << 2);
  }

  if (move.piece == KING && move.Is(CASTLE_LEFT)) [[unlikely]] {
    int king_square = BIT_INDEX(piece);
    Bitboard rank = RankMask(king_square);
    Bitboard &rooks = board_.pieces_[opp][ROOK];
    Bitboard rook = rooks & rank & kQueenSide;

    rooks ^= rook | (rook >> 3);
  }

  if (move.Is(EN_PASSANT)) [[unlikely]] {
    Bitboard *pieces = board_.pieces_[turn_];

    pieces[PAWN] |= move.ep_target;
  }

  if (opp == BLACK) {
    fullmove_counter_--;
  }

  _State state = history_.top();

  halfmove_clock_ = state.halfmove_clock;
  en_passant_sq_ = state.en_passant_square;
  castling_rights_ = state.castling_rights;

  turn_ = opp;

  history_.pop();
  moves_.pop_front();

  UpdateInternals();
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
