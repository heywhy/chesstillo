#include <chesstillo/fen.hpp>
#include <chesstillo/types.hpp>
#include <gtest/gtest.h>

class MoveGenTestSuite : public testing::Test {
protected:
  Board board;

  void SetUp() override { ApplyFen(board, START_FEN); }

  void TearDown() override { board.Reset(); }
};

TEST_F(MoveGenTestSuite, KingCantCapture) {
  ApplyFen(board,
           "r1bqkbnr/ppp2pp1/2np3p/8/2B1PQ2/8/PPPP2PP/RNB1K1NR w KQkq - 0 1");

  board.ApplyMove(
      {BitboardForSquare('f', 4), BitboardForSquare('f', 7), WHITE, QUEEN});

  ASSERT_EQ(PositionToFen(board),
            "r1bqkbnr/ppp2Qp1/2np3p/8/2B1P3/8/PPPP2PP/RNB1K1NR w KQkq - 0 1");
  ASSERT_TRUE(board.GetMoves().front().Is(CHECKMATE));
}
