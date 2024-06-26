#include <gtest/gtest.h>

#include "board.hpp"
#include "fen.hpp"
#include "move.hpp"

TEST(Board, ApplyPawnMove) {
  Board board;

  board = ApplyFen(board, START_FEN);

  Bitboard empty_squares = board.EmptySquares();

  Bitboard e2 = BitboardForSquare(e, 1);
  Bitboard e4 = BitboardForSquare(e, 3);

  Move move = {e2, e4, WHITE, PAWN};

  board.ApplyMove(move);

  ASSERT_NE(board.EmptySquares(), empty_squares);
  ASSERT_TRUE(board.Position(PAWN, WHITE) & e4);
}
