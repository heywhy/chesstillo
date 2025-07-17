#include <tuple>
#include <utility>

#include <engine/board.hpp>
#include <engine/constants.hpp>
#include <engine/fill.hpp>
#include <engine/hash.hpp>
#include <engine/move.hpp>
#include <engine/move_gen.hpp>
#include <engine/position.hpp>
#include <engine/square.hpp>
#include <engine/types.hpp>
#include <engine/utility.hpp>

namespace engine {
namespace position {

State State::From(Position &position) {
  return {position.king_ban_,
          position.board_.occupied_sqs,
          position.en_passant_sq_,
          position.en_passant_target_,
          position.castling_rights_,
          position.halfmove_clock_,
          position.hash_};
}

void State::Apply(Position &position, State &state) {
  position.king_ban_ = state.king_ban;
  position.halfmove_clock_ = state.halfmove_clock;
  position.castling_rights_ = state.castling_rights;
  position.en_passant_sq_ = state.en_passant_square;
  position.en_passant_target_ = state.en_passant_target;
  position.board_.occupied_sqs = state.occupied_sqs;

  position.hash_ = state.hash_;
}

}  // namespace position

Position::Position()
    : turn_(WHITE),
      hash_(0),
      king_ban_(kEmpty),
      en_passant_sq_(kEmpty),
      castling_rights_(0),
      fullmove_counter_(1),
      halfmove_clock_(0) {};

Position::Position(const Position &src) { Clone(src); }

Position &Position::operator=(const Position &src) {
  Clone(src);

  return *this;
}

void Position::Clone(const Position &src) {
  turn_ = src.turn_;
  king_ban_ = src.king_ban_;
  en_passant_sq_ = src.en_passant_sq_;
  en_passant_target_ = src.en_passant_target_;
  castling_rights_ = src.castling_rights_;
  fullmove_counter_ = src.fullmove_counter_;
  halfmove_clock_ = src.halfmove_clock_;

  board_ = src.board_;
  history_ = src.history_;
  mailbox_ = src.mailbox_;
  hash_ = src.hash_;
}

bool Position::PieceAt(Piece *piece, int index) const {
  *piece = mailbox_[index];

  return *piece != NONE;
}

bool Position::PieceAt(char *c, int index) const {
  Piece piece;

  if (!PieceAt(&piece, index)) {
    return false;
  }

  Bitboard bb = square::BB(index);

  if (board_.pieces[WHITE][piece] & bb) {
    return PieceToChar(c, piece, WHITE);
  }

  if (board_.pieces[BLACK][piece] & bb) {
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
  hash_ = 0;

  for (int i = 0; i < 64; i++) {
    mailbox_[i] = NONE;
  }
}

void Position::Make(const Move &move) {
  history_.push(position::State::From(*this));

  Color opp = OPP(turn_);
  Bitboard &piece = board_.pieces[turn_][move.piece];
  Bitboard to = square::BB(move.to);
  Bitboard from = square::BB(move.from);

  piece ^= from ^ to;

  mailbox_[move.from] = NONE;
  mailbox_[move.to] = move.piece;

  hash_ ^= kZobrist.color;
  hash_ ^= HASH2(move.from, move.to, turn_, move.piece);

  if (move.Is(move::CAPTURE)) {
    Bitboard &piece = board_.pieces[opp][move.captured];

    piece ^= to;
    hash_ ^= HASH1(move.to, opp, move.captured);
  }

  if (move.piece == KING && move.Is(move::CASTLE_KING_SIDE)) [[unlikely]] {
    int king_square = square::Index(piece);
    Bitboard rank = square::RankMask(king_square);
    Bitboard &rooks = board_.pieces[turn_][ROOK];
    Bitboard rook = rooks & rank & kKingSide;
    Bitboard new_position = rook >> 2;

    int old_index = square::Index(rook);
    int new_index = square::Index(new_position);

    rooks ^= rook | new_position;

    mailbox_[old_index] = NONE;
    mailbox_[new_index] = ROOK;

    hash_ ^= HASH2(old_index, new_index, turn_, ROOK);
  }

  if (move.piece == KING && move.Is(move::CASTLE_QUEEN_SIDE)) [[unlikely]] {
    int king_square = square::Index(piece);
    Bitboard rank = square::RankMask(king_square);
    Bitboard &rooks = board_.pieces[turn_][ROOK];
    Bitboard rook = rooks & rank & kQueenSide;
    Bitboard new_position = rook << 3;

    int old_index = square::Index(rook);
    int new_index = square::Index(new_position);

    rooks ^= rook | new_position;

    mailbox_[old_index] = NONE;
    mailbox_[new_index] = ROOK;

    hash_ ^= HASH2(old_index, new_index, turn_, ROOK);
  }

  auto [queen_side_rook, king_side_rook, queen_side_castling_flag,
        king_side_castling_flag, single_push] =
      turn_ == WHITE
          ? std::make_tuple(a1, h1, position::CASTLE_W_QUEEN_SIDE,
                            position::CASTLE_W_KING_SIDE, PushPawn<WHITE>)
          : std::make_tuple(a8, h8, position::CASTLE_B_QUEEN_SIDE,
                            position::CASTLE_B_KING_SIDE, PushPawn<BLACK>);

  {
    if ((move.piece == KING ||
         (move.piece == ROOK && move.from == queen_side_rook)) &&
        castling_rights_ & queen_side_castling_flag) [[unlikely]] {
      int index = square::Index(queen_side_castling_flag);

      castling_rights_ ^= queen_side_castling_flag;
      hash_ ^= kZobrist.castling_rights[index];
    }

    if ((move.piece == KING ||
         (move.piece == ROOK && move.from == king_side_rook)) &&
        castling_rights_ & king_side_castling_flag) [[unlikely]] {
      int index = square::Index(queen_side_castling_flag);

      castling_rights_ ^= king_side_castling_flag;
      hash_ ^= kZobrist.castling_rights[index];
    }
  }

  if (move.Is(move::EN_PASSANT)) [[unlikely]] {
    Bitboard &piece = board_.pieces[opp][PAWN];
    int index = square::Index(en_passant_target_);

    piece ^= en_passant_target_;

    mailbox_[index] = NONE;
    hash_ ^=
        HASH1(index, opp, PAWN) ^ kZobrist.en_passant_file[square::File(index)];
  }

  if (move.Is(move::PROMOTION)) [[unlikely]] {
    Bitboard &new_piece = board_.pieces[turn_][move.promoted];

    // unset the old piece & set the index on the promoted piece
    piece ^= to;
    new_piece ^= to;

    mailbox_[move.to] = move.promoted;

    // INFO: unset the pawn move before the promotion.
    hash_ ^= HASH1(move.to, turn_, move.piece);
    hash_ ^= HASH1(move.to, turn_, move.promoted);
  }

  if (en_passant_sq_) {
    int index = square::Index(en_passant_sq_);
    int file = square::File(index);

    hash_ ^= kZobrist.en_passant_file[file];
  }

  turn_ = opp;
  en_passant_sq_ = kEmpty;

  bool is_double_push =
      ((from & kRank2) | (from & kRank7)) && ((to & kRank4) | (to & kRank5));

  if (move.piece == PAWN && is_double_push) {
    en_passant_sq_ = single_push(from);
    int index = square::Index(en_passant_sq_);
    int file = square::File(index);

    hash_ ^= kZobrist.en_passant_file[file];
  }

  if (move.piece == PAWN || move.Is(move::CAPTURE) ||
      move.Is(move::EN_PASSANT)) {
    halfmove_clock_ = 0;
  } else {
    halfmove_clock_++;
  }

  if (opp == WHITE) {
    fullmove_counter_++;
  }

  UpdateInternals();
}

void Position::Undo(const Move &move) {
  position::State::Apply(*this, history_.top());

  Color opp = OPP(turn_);
  Bitboard &piece = board_.pieces[opp][move.piece];
  Bitboard to = square::BB(move.to);
  Bitboard from = square::BB(move.from);

  piece = (piece ^ to) | from;

  mailbox_[move.from] = move.piece;

  if (move.Is(move::PROMOTION)) [[unlikely]] {
    Bitboard &new_piece = board_.pieces[opp][move.promoted];

    // reset the old piece & unset the promoted piece
    piece ^= to;
    new_piece ^= to;

    mailbox_[move.to] = NONE;
  }

  if (move.Is(move::CAPTURE)) {
    PieceList &pieces = board_.pieces[turn_];

    pieces[move.captured] |= to;

    mailbox_[move.to] = move.captured;
  }

  if (move.piece == KING && move.Is(move::CASTLE_KING_SIDE)) [[unlikely]] {
    int king_square = square::Index(piece);
    Bitboard rank = square::RankMask(king_square);
    Bitboard &rooks = board_.pieces[opp][ROOK];
    Bitboard rook = rooks & rank & kKingSide;
    Bitboard old_position = (rook << 2);

    rooks ^= rook | old_position;

    mailbox_[square::Index(rook)] = NONE;
    mailbox_[square::Index(old_position)] = ROOK;
  }

  if (move.piece == KING && move.Is(move::CASTLE_QUEEN_SIDE)) [[unlikely]] {
    int king_square = square::Index(piece);
    Bitboard rank = square::RankMask(king_square);
    Bitboard &rooks = board_.pieces[opp][ROOK];
    Bitboard rook = rooks & rank & kQueenSide;
    Bitboard old_position = (rook >> 3);

    rooks ^= rook | old_position;

    mailbox_[square::Index(rook)] = NONE;
    mailbox_[square::Index(old_position)] = ROOK;
  }

  if (move.Is(move::EN_PASSANT)) [[unlikely]] {
    PieceList &pieces = board_.pieces[turn_];

    pieces[PAWN] |= en_passant_target_;

    mailbox_[square::Index(en_passant_target_)] = PAWN;
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
  UpdateEnPassantSq();
  UpdateMailbox();
}

void Position::UpdateKingBan() {
  king_ban_ = kEmpty;
  Color opp = OPP(turn_);

  Bitboard enemy_pawns = board_.pieces[opp][PAWN];
  Bitboard enemy_bishop_queen =
      board_.pieces[opp][BISHOP] | board_.pieces[opp][QUEEN];
  Bitboard enemy_rook_queen =
      board_.pieces[opp][ROOK] | board_.pieces[opp][QUEEN];

  auto [pawn_east_targets, pawn_west_targets] =
      turn_ == WHITE
          ? std::make_pair(PawnTargets<BLACK, EAST>, PawnTargets<BLACK, WEST>)
          : std::make_pair(PawnTargets<WHITE, EAST>, PawnTargets<WHITE, WEST>);

  king_ban_ |=
      (KING_ATTACKS(board_.pieces[opp][KING])) |
      (KNIGHT_ATTACKS(board_.pieces[opp][KNIGHT])) |
      (BISHOP_ATTACKS(enemy_bishop_queen,
                      ~board_.occupied_sqs | board_.pieces[turn_][KING])) |
      (ROOK_ATTACKS(enemy_rook_queen,
                    ~board_.occupied_sqs | board_.pieces[turn_][KING])) |
      pawn_east_targets(enemy_pawns) | pawn_west_targets(enemy_pawns);
}

void Position::UpdateEnPassantSq() {
  en_passant_target_ = (PushPawn<WHITE>(en_passant_sq_) & kRank4) |
                       (PushPawn<BLACK>(en_passant_sq_) & kRank5);

  Color opp = OPP(turn_);
  Bitboard enemy_rook_queen =
      board_.pieces[opp][ROOK] | board_.pieces[opp][QUEEN];

  Bitboard pawns = board_.pieces[turn_][PAWN];
  Bitboard king = board_.pieces[turn_][KING];
  int ep_sq = square::Index(en_passant_target_);
  Bitboard ep_rank = square::RankMask(ep_sq);
  int king_sq = square::Index(king);

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
          board_.occupied_sqs & ~(en_passant_target_ | west_targets);

      if (kSlidingAttacks.Rook(occupied_sqs_after_ep, king_sq) &
          enemy_rook_queen) {
        en_passant_sq_ = kEmpty;
      }
    }

    if (east_targets) {
      Bitboard occupied_sqs_after_ep =
          board_.occupied_sqs & ~(en_passant_target_ | east_targets);

      if (kSlidingAttacks.Rook(occupied_sqs_after_ep, king_sq) &
          enemy_rook_queen) {
        en_passant_sq_ = kEmpty;
      }
    }
  }
}

void Position::UpdateMailbox() {
  for (int i = 0; i < 64; i++) {
    Bitboard bb = square::BB(i);

    if (bb & board_.occupied_sqs) {
      for (int j = 0; j < PIECES; j++) {
        Piece piece = static_cast<Piece>(j);

        if ((board_.pieces[WHITE][j] & bb) | (board_.pieces[BLACK][j] & bb)) {
          mailbox_[i] = piece;
        }
      }
    } else {
      mailbox_[i] = NONE;
    }
  }
}

}  // namespace engine
