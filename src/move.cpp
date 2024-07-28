#include <cstdarg>
#include <cstddef>
#include <cstdint>
#include <vector>

#include <chesstillo/constants.hpp>
#include <chesstillo/move.hpp>
#include <chesstillo/types.hpp>
#include <chesstillo/utility.hpp>

#define MAX_MOVES_BUFFER_SIZE 256

void SetFlags(Move *move, ...) {
  std::va_list args;
  unsigned int flag;

  va_start(args, move);

  while ((flag = va_arg(args, unsigned int))) {
    move->flags |= static_cast<std::uint8_t>(1) << flag;
  }

  va_end(args);
}

Bitboard BishopAttacks(Bitboard piece) {
  int square = SquareFromBitboard(piece);

  return DiagonalMask(square) ^ AntiDiagonalMask(square);
}

Bitboard RookAttacks(Bitboard piece) {
  int square = SquareFromBitboard(piece);

  return RankMask(square) ^ FileMask(square);
}

Bitboard QueenAttacks(Bitboard piece) {
  int square = SquareFromBitboard(piece);

  return (FileMask(square) | RankMask(square)) ^
         (DiagonalMask(square) | AntiDiagonalMask(square));
}

std::vector<Move> GenerateMoves(Board &board, Piece piece) {
  std::vector<Move> moves;

  Bitboard &bb =
      board.turn_ == WHITE ? board.w_pieces_[piece] : board.b_pieces_[piece];

  Bitboard squares[MAX_MOVES_BUFFER_SIZE];
  Bitboard targets[MAX_MOVES_BUFFER_SIZE];

  std::size_t size = Split(bb, squares);

  for (std::size_t i = 0; i < size; i++) {
    Bitboard square = squares[i];
    std::size_t size = GenerateMoves(board, piece, square, targets);

    for (std::size_t j = 0; j < size; j++) {
      moves.emplace_back(squares[i], targets[j], board.turn_, piece);
    }
  }

  return moves;
}

std::size_t GenerateMoves(Board &board, Piece piece, Bitboard square,
                          Bitboard *const out) {
  std::size_t size = 0;

  switch (piece) {
  case PAWN:
    size = GeneratePawnMoves(board, square, out);
    break;

  case KNIGHT:
    size = GenerateKnightMoves(board, square, out);
    break;

  case KING:
    size = GenerateKingMoves(board, square, out);
    break;

  case BISHOP:
  case ROOK:
  case QUEEN:
    size = GenerateSlidingPieceMoves(board, piece, square, out);
    break;
  }

  return size;
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

std::size_t GeneratePawnMoves(Board &board, Bitboard square,
                              Bitboard *const out) {
  Bitboard targets;
  Bitboard single_push;
  Bitboard empty_sqs = ~board.occupied_sqs_;
  Bitboard attacked_sqs = board.turn_ == WHITE ? board.sqs_occupied_by_b_
                                               : board.sqs_occupied_by_w_;

  if (board.turn_ == WHITE) {
    single_push = MOVE_NORTH(square) & empty_sqs;

    targets = single_push | (MOVE_NORTH(single_push) & empty_sqs & kRank4) |
              (MOVE_NORTH_EAST(square) & attacked_sqs) |
              (MOVE_NORTH_WEST(square) & attacked_sqs);
  } else {
    single_push = MOVE_SOUTH(square) & empty_sqs;

    targets = single_push | (MOVE_SOUTH(single_push) & empty_sqs & kRank5) |
              (MOVE_SOUTH_EAST(square) & attacked_sqs) |
              (MOVE_SOUTH_WEST(square) & attacked_sqs);
  }

  return Split(targets, out);
}

std::size_t GenerateKnightMoves(Board &board, Bitboard square,
                                Bitboard *const out) {
  Bitboard targets = KNIGHT_ATTACKS(square);
  Bitboard occupied_sqs = board.turn_ == WHITE ? board.sqs_occupied_by_w_
                                               : board.sqs_occupied_by_b_;

  return Split((targets ^ occupied_sqs) & targets, out);
}

std::size_t GenerateSlidingPieceMoves(Board &board, Piece piece,
                                      Bitboard square, Bitboard *const out) {
  Bitboard targets;
  Bitboard occupied_sqs = board.turn_ == WHITE ? board.sqs_occupied_by_w_
                                               : board.sqs_occupied_by_b_;

  switch (piece) {
  case BISHOP:
    targets = BishopAttacks(square);
    break;

  case QUEEN:
    targets = QueenAttacks(square);
    break;

  case ROOK:
    targets = RookAttacks(square);
    break;

  default:
    return 0;
  }

  Bitboard allowed = kEmpty;
  Bitboard tmp[MAX_MOVES_BUFFER_SIZE];
  std::size_t size = Split(targets, tmp);

  for (std::size_t i = 0; i < size; i++) {
    Bitboard destination = tmp[i];
    Bitboard squares_btwn = SquaresInBetween(square, destination);

    if (squares_btwn & board.occupied_sqs_) {
      continue;
    }

    allowed |= destination;
  }

  // 1. cancel out all occupied squares
  // 2. and filter only squares in targets
  return Split((allowed ^ occupied_sqs) & allowed, out);
}

std::size_t GenerateKingMoves(Board &board, Bitboard square,
                              Bitboard *const out) {
  Bitboard targets = KING_ATTACKS(square);
  Bitboard occupied_sqs = board.turn_ == WHITE ? board.sqs_occupied_by_w_
                                               : board.sqs_occupied_by_b_;

  return Split((targets ^ occupied_sqs) & targets, out);
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
