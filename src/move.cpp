#include <cstdarg>

#include "constants.hpp"
#include "move.hpp"
#include "types.hpp"

void SetFlags(Move *move, ...) {
  std::va_list args;
  unsigned int flag;

  va_start(args, move);

  while ((flag = va_arg(args, unsigned int))) {
    move->flags |= 1U << flag;
  }

  va_end(args);
}

Bitboard BishopAttacks(Bitboard piece) {
  int square = SquareFromBitboard(piece);
  Bitboard attacked_sqs = DiagonalMask(square) | AntiDiagonalMask(square);

  return ExcludeSquareBit(attacked_sqs, square);
}

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

Bitboard SquaresInBetween(Bitboard from, Bitboard to) {
  static Bitboard const M = -1;
  Bitboard btwn, line, rank, file;

  int toSq = SquareFromBitboard(to);
  int fromSq = SquareFromBitboard(from);

  btwn = (M << fromSq) ^ (M << toSq);
  file = (toSq & 7) - (fromSq & 7);
  rank = ((toSq | 7) - fromSq) >> 3;
  line = ((file & 7) - 1) & kA2A7;

  line += 2 * (((rank & 7) - 1) >> 58);
  line += (((rank - file) & 15) - 1) & kB2G7;
  line += (((rank + file) & 15) - 1) & kH1B7;
  line *= btwn & -btwn;

  return line & btwn;
}

bool IsValidBishopMove(Board &board, const Bitboard piece, const Move &move,
                       const Bitboard &attacking_sqs) {
  Bitboard targets = BishopAttacks(piece & move.from);
  Bitboard squares_btwn = SquaresInBetween(move.from, move.to);

  if (squares_btwn & board.occupied_sqs_)
    return false;

  return targets & move.to;
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

Bitboard RankMask(int square) { return kRank1 << (square & 56); }

Bitboard FileMask(int square) { return kAFile << (square & 7); }

Bitboard DiagonalMask(int square) {
  int diagonal = (square & 7) - (square >> 3);

  return diagonal >= 0 ? kA1H8Diagonal >> diagonal * 8
                       : kA1H8Diagonal << -diagonal * 8;
}

Bitboard AntiDiagonalMask(int square) {
  int diagonal = 7 - (square & 7) - (square >> 3);

  return diagonal >= 0 ? kH1A8Diagonal >> diagonal * 8
                       : kH1A8Diagonal << -diagonal * 8;
}

Bitboard ExcludeSquareBit(Bitboard bb, int square) {
  return (static_cast<Bitboard>(1) << square) ^ bb;
}
