#include <chesstillo/board.hpp>
#include <chesstillo/constants.hpp>
#include <chesstillo/fen.hpp>
#include <chesstillo/fill.hpp>
#include <chesstillo/move_gen.hpp>
#include <chesstillo/position.hpp>
#include <chesstillo/types.hpp>
#include <chesstillo/utility.hpp>
#include <cstdint>
#include <tuple>
#include <utility>

_State _State::From(Position &position) {
  return {position.king_ban_,        *position.occupied_sqs_,
          position.en_passant_sq_,   position.en_passant_target_,
          position.castling_rights_, position.halfmove_clock_};
}

void _State::Apply(Position &position, _State &state) {
  position.king_ban_ = state.king_ban;
  *position.occupied_sqs_ = state.occupied_sqs;
  position.halfmove_clock_ = state.halfmove_clock;
  position.castling_rights_ = state.castling_rights;
  position.en_passant_sq_ = state.en_passant_square;
  position.en_passant_target_ = state.en_passant_target;
}

bool Position::PieceAt(Piece *piece, uint8_t index) const {
  *piece = mailbox_[index];

  return *piece != NONE;
}

bool Position::PieceAt(char *c, uint8_t index) const {
  Piece piece;

  if (!PieceAt(&piece, index)) {
    return false;
  }

  Bitboard bb = BITBOARD_FOR_SQUARE(index);

  if (board_.pieces_[WHITE][piece] & bb) {
    return PieceToChar(c, piece, WHITE);
  }

  if (board_.pieces_[BLACK][piece] & bb) {
    return PieceToChar(c, piece, BLACK);
  }

  return false;
}

void Position::Reset() {
  board_.Reset();

  king_ban_ = kEmpty;
  castling_rights_ = 0;
  en_passant_sq_ = kEmpty;
  history_ = kEmptyStack;

  for (int i = 0; i < 64; i++) {
    mailbox_[i] = NONE;
  }
}

void Position::Make(Move &move) {
  history_.push(_State::From(*this));

  Color opp = OPP(turn_);
  Bitboard &piece = board_.pieces_[turn_][move.piece];
  Bitboard to = BITBOARD_FOR_SQUARE(move.to);
  Bitboard from = BITBOARD_FOR_SQUARE(move.from);

  piece ^= from ^ to;

  mailbox_[move.from] = NONE;
  mailbox_[move.to] = move.piece;

  if (move.Is(CAPTURE)) {
    Bitboard &piece = board_.pieces_[opp][move.captured];

    piece ^= to;
  }

  if (move.piece == KING && move.Is(CASTLE_RIGHT)) [[unlikely]] {
    int king_square = BIT_INDEX(piece);
    Bitboard rank = RankMask(king_square);
    Bitboard &rooks = board_.pieces_[turn_][ROOK];
    Bitboard rook = rooks & rank & kKingSide;
    Bitboard new_position = rook >> 2;

    rooks ^= rook | new_position;

    mailbox_[BIT_INDEX(rook)] = NONE;
    mailbox_[BIT_INDEX(new_position)] = ROOK;
  }

  if (move.piece == KING && move.Is(CASTLE_LEFT)) [[unlikely]] {
    int king_square = BIT_INDEX(piece);
    Bitboard rank = RankMask(king_square);
    Bitboard &rooks = board_.pieces_[turn_][ROOK];
    Bitboard rook = rooks & rank & kQueenSide;
    Bitboard new_position = rook << 3;

    rooks ^= rook | new_position;

    mailbox_[BIT_INDEX(rook)] = NONE;
    mailbox_[BIT_INDEX(new_position)] = ROOK;
  }

  auto [left_rook, right_rook, single_push] =
      turn_ == WHITE ? std::make_tuple(a1, h1, PushPawn<WHITE>)
                     : std::make_tuple(a8, h8, PushPawn<BLACK>);

  {
    Bitboard castle_left = CASTLE_LEFT(turn_);
    Bitboard castle_right = CASTLE_RIGHT(turn_);

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

    piece ^= en_passant_target_;

    mailbox_[BIT_INDEX(en_passant_target_)] = NONE;
  }

  if (move.Is(PROMOTION)) [[unlikely]] {
    Bitboard &new_piece = board_.pieces_[turn_][move.promoted];

    // unset the old piece & set the index on the promoted piece
    piece ^= to;
    new_piece ^= to;

    mailbox_[move.to] = move.promoted;
  }

  turn_ = opp;
  en_passant_sq_ = kEmpty;

  bool is_double_push =
      ((from & kRank2) | (from & kRank7)) && ((to & kRank4) | (to & kRank5));

  if (move.piece == PAWN && is_double_push) {
    en_passant_sq_ = single_push(from);
  }

  if (move.piece == PAWN || move.Is(CAPTURE)) {
    halfmove_clock_ = 0;
  } else {
    halfmove_clock_++;
  }

  if (opp == WHITE) {
    fullmove_counter_++;
  }

  UpdateInternals();
}

void Position::Undo(Move &move) {
  _State::Apply(*this, history_.top());

  Color opp = OPP(turn_);
  Bitboard &piece = board_.pieces_[opp][move.piece];
  Bitboard to = BITBOARD_FOR_SQUARE(move.to);
  Bitboard from = BITBOARD_FOR_SQUARE(move.from);

  piece = (piece ^ to) | from;

  mailbox_[move.from] = move.piece;

  if (move.Is(PROMOTION)) [[unlikely]] {
    Bitboard &new_piece = board_.pieces_[opp][move.promoted];

    // reset the old piece & unset the promoted piece
    piece ^= to;
    new_piece ^= to;

    mailbox_[move.to] = NONE;
  }

  if (move.Is(CAPTURE)) {
    Bitboard *pieces = board_.pieces_[turn_];

    pieces[move.captured] |= to;

    mailbox_[move.to] = move.captured;
  }

  if (move.piece == KING && move.Is(CASTLE_RIGHT)) [[unlikely]] {
    uint8_t king_square = BIT_INDEX(piece);
    Bitboard rank = RankMask(king_square);
    Bitboard &rooks = board_.pieces_[opp][ROOK];
    Bitboard rook = rooks & rank & kKingSide;
    Bitboard old_position = (rook << 2);

    rooks ^= rook | old_position;

    mailbox_[BIT_INDEX(rook)] = NONE;
    mailbox_[BIT_INDEX(old_position)] = ROOK;
  }

  if (move.piece == KING && move.Is(CASTLE_LEFT)) [[unlikely]] {
    uint8_t king_square = BIT_INDEX(piece);
    Bitboard rank = RankMask(king_square);
    Bitboard &rooks = board_.pieces_[opp][ROOK];
    Bitboard rook = rooks & rank & kQueenSide;
    Bitboard old_position = (rook >> 3);

    rooks ^= rook | old_position;

    mailbox_[BIT_INDEX(rook)] = NONE;
    mailbox_[BIT_INDEX(old_position)] = ROOK;
  }

  if (move.Is(EN_PASSANT)) [[unlikely]] {
    Bitboard *pieces = board_.pieces_[turn_];

    pieces[PAWN] |= en_passant_target_;

    mailbox_[BIT_INDEX(en_passant_target_)] = PAWN;
  }

  if (opp == BLACK) {
    fullmove_counter_--;
  }

  turn_ = opp;

  history_.pop();
}

void Position::UpdateInternals() {
  board_.UpdateOccupiedSqs();

  UpdateKingBan();

  en_passant_target_ = (PushPawn<WHITE>(en_passant_sq_) & kRank4) |
                       (PushPawn<BLACK>(en_passant_sq_) & kRank5);

  Color opp = OPP(turn_);
  Bitboard enemy_rook_queen =
      board_.pieces_[opp][ROOK] | board_.pieces_[opp][QUEEN];

  Bitboard pawns = board_.pieces_[turn_][PAWN];
  Bitboard king = board_.pieces_[turn_][KING];
  uint8_t ep_sq = BIT_INDEX(en_passant_target_);
  Bitboard ep_rank = RankMask(ep_sq);
  uint8_t king_sq = BIT_INDEX(king);

  if (en_passant_target_ && (ep_rank & king) && (ep_rank & enemy_rook_queen) &&
      (ep_rank & pawns)) {
    auto [pawn_west_targets, pawn_east_targets] =
        turn_ == WHITE
            ? std::make_pair(PawnTargets<BLACK, WEST>, PawnTargets<BLACK, EAST>)
            : std::make_pair(PawnTargets<WHITE, WEST>,
                             PawnTargets<WHITE, EAST>);

    Bitboard west_targets = pawn_west_targets(en_passant_sq_) & pawns;
    Bitboard east_targets = pawn_east_targets(en_passant_sq_) & pawns;

    if (west_targets) {
      Bitboard occupied_sqs_after_ep =
          *occupied_sqs_ & ~(en_passant_target_ | west_targets);

      if (kSlidingAttacks.Rook(occupied_sqs_after_ep, king_sq) &
          enemy_rook_queen) {
        en_passant_sq_ = kEmpty;
      }
    }

    if (east_targets) {
      Bitboard occupied_sqs_after_ep =
          *occupied_sqs_ & ~(en_passant_target_ | east_targets);

      if (kSlidingAttacks.Rook(occupied_sqs_after_ep, king_sq) &
          enemy_rook_queen) {
        en_passant_sq_ = kEmpty;
      }
    }
  }
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
          ? std::make_pair(PawnTargets<BLACK, EAST>, PawnTargets<BLACK, WEST>)
          : std::make_pair(PawnTargets<WHITE, EAST>, PawnTargets<WHITE, WEST>);

  king_ban_ |=
      (KING_ATTACKS(board_.pieces_[opp][KING])) |
      (KNIGHT_ATTACKS(board_.pieces_[opp][KNIGHT])) |
      (BISHOP_ATTACKS(enemy_bishop_queen,
                      ~board_.occupied_sqs_ | board_.pieces_[turn_][KING])) |
      (ROOK_ATTACKS(enemy_rook_queen,
                    ~board_.occupied_sqs_ | board_.pieces_[turn_][KING])) |
      pawn_east_targets(enemy_pawns) | pawn_west_targets(enemy_pawns);
}

void Position::UpdateMailbox() {
  for (int i = 0; i < 64; i++) {
    Bitboard bb = BITBOARD_FOR_SQUARE(i);

    if (bb & *occupied_sqs_) {
      for (int j = 0; j < PIECES; j++) {
        Piece piece = static_cast<Piece>(j);

        if ((board_.pieces_[WHITE][j] & bb) | (board_.pieces_[BLACK][j] & bb)) {
          mailbox_[i] = piece;
        }
      }
    } else {
      mailbox_[i] = NONE;
    }
  }
}
