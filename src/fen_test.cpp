#include <gtest/gtest.h>

#include "fen.hpp"

TEST(FenTest, ApplyFen) {
  Board board;

  ASSERT_EQ(~board.EmptySquares(), EMPTY);

  board = ApplyFen(board, START_FEN);

  ASSERT_NE(~board.EmptySquares(), EMPTY);
}

TEST(FenTest, PositionToFen) {
  Board board;

  board = ApplyFen(board, START_FEN);

  ASSERT_EQ(PositionToFen(board), START_FEN);

  Bitboard e2 = BitboardForSquare(e, 1);
  Bitboard e4 = BitboardForSquare(e, 3);
  Move white_move = {e2, e4, WHITE, PAWN};

  Bitboard c7 = BitboardForSquare(c, 6);
  Bitboard c5 = BitboardForSquare(c, 4);
  Move black_move = {c7, c5, BLACK, PAWN};

  board.ApplyMove(white_move);
  board.ApplyMove(black_move);

  ASSERT_EQ(PositionToFen(board),
            "rnbqkbnr/pp1ppppp/8/2p5/4P3/8/PPPP1PPP/RNBQKBNR w KQkq - 0 1");
}
