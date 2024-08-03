#include <cstdarg>
#include <cstddef>
#include <iterator>
#include <vector>

#include <chesstillo/constants.hpp>
#include <chesstillo/move.hpp>
#include <chesstillo/types.hpp>
#include <chesstillo/utility.hpp>

#include "fill.hpp"

// The max number of squares any piece can travel to is that of the queen
#define MAX_PIECE_MOVES 27
// Assuming the maximum number of any piece type asides king &
// pawns is 10
#define MAX_PIECE_NUMBER 10
#define MAX_MOVES_BUFFER_SIZE 256

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

std::vector<Move> GenerateOutOfCheckMoves(Board &board) {
  std::vector<Move> moves;
  Color opp = static_cast<Color>(board.turn_ ^ 1);
  Bitboard king_sq = board.pieces_[board.turn_][KING];
  Bitboard king_sq_color =
      king_sq & kDarkSquares ? kDarkSquares : kLightSquares;

  for (int piece = 0; piece < 6; piece++) {
    if (piece == KING || !(board.attacking_sqs_[opp][piece] & king_sq)) {
      continue;
    }

    Bitboard opp_pieces = board.pieces_[opp][piece];

    if ((piece == KNIGHT || piece == BISHOP) && !(opp_pieces & king_sq_color)) {
      continue;
    }

    /**
     * A knight or bishop can only attack a king if they're on the same
     * square color, so we take advantage of this fact to filter only pieces on
     * same square color.
     */
    if (piece == KNIGHT || piece == BISHOP) {
      opp_pieces &= king_sq_color;
    }

    Bitboard sqs_btwn_king_and_attacker = kEmpty;

    /**
     * Here, we grab the squares in between the sliding piece and the
     * king so that we can generate moves that blocks the attack path in
     * situations we are unable to capture the attacker.
     */
    if (piece == BISHOP || piece == ROOK || piece == QUEEN) {
      sqs_btwn_king_and_attacker =
          SquaresInBetween(opp_pieces, king_sq) & ~board.occupied_sqs_;
    }

    for (int i = 0; i < 6; i++) {
      if (i == KING) {
        continue;
      }

      std::size_t moves_num = 0;
      Bitboard squares[MAX_PIECE_NUMBER];
      Bitboard tmp_moves[MAX_PIECE_MOVES];
      Piece piece = static_cast<Piece>(i);
      std::size_t count = Split(board.pieces_[board.turn_][piece], squares);

      for (std::size_t i = 0; i < count; i++) {
        Bitboard square = squares[i];

        switch (piece) {
        case PAWN:
          moves_num = GeneratePawnMoves(board, square, tmp_moves);
          break;

        case KNIGHT:
          moves_num = GenerateKnightMoves(board, square, tmp_moves);
          break;

        case BISHOP:
        case ROOK:
        case QUEEN:
          moves_num =
              GenerateSlidingPieceMoves(board, piece, square, tmp_moves);
          break;

        default:
          break;
        }

        for (std::size_t j = 0; j < moves_num; j++) {
          Bitboard destination = tmp_moves[j];

          if (destination & opp_pieces ||
              destination & sqs_btwn_king_and_attacker) {
            moves.emplace_back(square, destination, board.turn_, piece);
          }
        }
      }
    }
  }

  // A king can only have 8 maximum number of squares per move.
  Bitboard targets[8];
  std::size_t size = GenerateKingMoves(board, king_sq, targets);

  for (int i = 0; i < size; i++) {
    // INFO: Instead of creating a copy of the board maybe an attack & defend
    // map can be used?!
    Board copy = board;
    Bitboard destination = targets[i];
    Move move(king_sq, destination, board.turn_, KING);

    copy.MakeMove(move);

    if (destination & copy.sqs_attacked_by_[opp]) {
      continue;
    }

    moves.push_back(std::move(move));
  }

  return moves;
}

std::vector<Move> GenerateMoves(Board &board) {
  if (board.sqs_attacked_by_[board.turn_ ^ 1] &
      board.pieces_[board.turn_][KING]) {
    return GenerateOutOfCheckMoves(board);
  }

  std::vector<Move> moves;

  for (int i = 0; i < 6; i++) {
    std::vector<Move> result = GenerateMoves(board, static_cast<Piece>(i));

    moves.insert(moves.end(), std::make_move_iterator(result.begin()),
                 std::make_move_iterator(result.end()));
  }

  return moves;
}

std::vector<Move> GenerateMoves(Board &board, Piece piece) {
  Color opp = static_cast<Color>(board.turn_ ^ 1);

  if (board.sqs_attacked_by_[opp] & board.pieces_[board.turn_][KING]) {
    return GenerateOutOfCheckMoves(board);
  }

  std::vector<Move> moves;
  Bitboard bb = board.pieces_[board.turn_][piece];

  Bitboard targets[MAX_PIECE_MOVES];
  Bitboard squares[MAX_PIECE_NUMBER];

  std::size_t size = Split(bb, squares);

  for (std::size_t i = 0; i < size; i++) {
    Bitboard square = squares[i];
    std::size_t size = GenerateMoves(board, piece, square, targets);

    for (std::size_t j = 0; j < size; j++) {
      // INFO: Maybe not copying the board is possible
      Board copy = board;
      Move move(squares[i], targets[j], board.turn_, piece);

      copy.MakeMove(move);

      if (copy.sqs_attacked_by_[opp] & copy.pieces_[board.turn_][KING]) {
        continue;
      }

      moves.push_back(std::move(move));
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
  Bitboard attacked_sqs = board.sqs_occupied_by_[board.turn_ ^ 1];

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
  Bitboard occupied_sqs = board.sqs_occupied_by_[board.turn_];

  return Split((targets ^ occupied_sqs) & targets, out);
}

std::size_t GenerateSlidingPieceMoves(Board &board, Piece piece,
                                      Bitboard square, Bitboard *const out) {
  Bitboard targets;
  Bitboard occupied_sqs = board.sqs_occupied_by_[board.turn_];

  switch (piece) {
  case BISHOP:
    targets = BISHOP_ATTACKS(square, ~board.occupied_sqs_);
    break;

  case QUEEN:
    targets = QUEEN_ATTACKS(square, ~board.occupied_sqs_);
    break;

  case ROOK:
    targets = ROOK_ATTACKS(square, ~board.occupied_sqs_);
    break;

  default:
    return 0;
  }

  // 1. cancel out all occupied squares
  // 2. and filter only squares in targets
  return Split((targets ^ occupied_sqs) & targets, out);
}

std::size_t GenerateKingMoves(Board &board, Bitboard square,
                              Bitboard *const out) {
  Bitboard targets = KING_ATTACKS(square);
  Bitboard occupied_sqs = board.sqs_occupied_by_[board.turn_];
  Bitboard sqs_attacked_by_opp = board.sqs_attacked_by_[board.turn_ ^ 1];

  targets &= (targets ^ occupied_sqs);

  return Split(targets & ~sqs_attacked_by_opp, out);
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
