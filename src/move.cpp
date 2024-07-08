#include "move.hpp"
#include "constants.hpp"
#include "types.hpp"
#include <mach/mach_init.h>

Bitboard GenPawnMoves(Color color, Bitboard position, Bitboard occupied_sqs,
                      Bitboard attacked_sqs_);

bool IsValidPawnMove(Board &board, Bitboard const piece, Move const &move,
                     Bitboard const &attacking_sqs) {
  Bitboard occupied_by_opp =
      move.color == WHITE ? board.sqs_occupied_by_b_ : board.sqs_occupied_by_w_;

  Bitboard attacked_sqs = attacking_sqs & occupied_by_opp;

  Bitboard targets =
      GenPawnMoves(move.color, piece, board.occupied_sqs_, attacked_sqs);

  return targets & move.to;
}

bool IsValidKnightMove(Board &, Bitboard const, Move const &move,
                       Bitboard const &attacking_sqs) {
  return attacking_sqs & move.to;
}

Bitboard GenPawnMoves(Color color, Bitboard piece, Bitboard occupied_sqs,
                      Bitboard attacked_sqs_) {
  Bitboard single_push;
  Bitboard empty_sqs = ~occupied_sqs;

  if (color == WHITE) {
    single_push = MOVE_NORTH(piece) & empty_sqs;

    return single_push | (MOVE_NORTH(single_push) & empty_sqs & kRank4) |
           (MOVE_NORTH_EAST(piece) & attacked_sqs_) |
           (MOVE_NORTH_WEST(piece) & attacked_sqs_);
  }

  single_push = MOVE_SOUTH(piece) & empty_sqs;

  return single_push | (MOVE_SOUTH(single_push) & empty_sqs & kRank5) |
         (MOVE_SOUTH_EAST(piece) & attacked_sqs_) |
         (MOVE_SOUTH_WEST(piece) & attacked_sqs_);
}
