#include <cstdio>
#include <vector>

#include <chesstillo/fen.hpp>
#include <chesstillo/move_gen.hpp>
#include <chesstillo/position.hpp>
#include <chesstillo/search.hpp>
#include <chesstillo/transposition.hpp>
#include <chesstillo/utility.hpp>
#include <gtest/gtest.h>

class SearchTestSuite : public testing::Test {
protected:
  TT tt;
  Position position;
  SearchManager manager;

  SearchTestSuite() : tt(MAX_TRANSPOSITION_SIZE), manager(&tt) {};

  void SetUp() override { ApplyFen(position, START_FEN); }

  void TearDown() override { position.Reset(); }
};

TEST_F(SearchTestSuite, OrderMoves) {
  // ApplyFen(position, "6k1/5pp1/4p3/1p1n2Qp/3P3P/BqP3P1/1P3PK1/8 w - - 0 1");
  // ApplyFen(position, "8/8/3N4/2r5/1p2RN2/1rk5/2p5/2K5 w - - 0 1");

  // std::vector<Move> moves = GenerateMoves(position);
  //
  // for (Move &move : moves) {
  //   char s[6];
  //
  //   if (ToString(s, move)) {
  //     std::printf("%s\n", s);
  //   }
  // }
  //
  // OrderMoves(moves, position);
  //
  // std::printf("===================\n");
  //
  // for (Move &move : moves) {
  //   char s[6];
  //
  //   if (ToString(s, move)) {
  //     std::printf("%s\n", s);
  //   }
  // }
  //
  // std::printf("===================\n");
}

TEST_F(SearchTestSuite, FindBestMove) {
  // ApplyFen(position, "6k1/5pp1/4p3/1p1n2Qp/3P3P/BqP3P1/1P3PK1/8 w - - 0 1");
  ApplyFen(position, "8/8/3N4/2r5/1p2RN2/1rk5/2p5/2K5 w - - 0 1");

  Move move = manager.FindBestMove(position);

  ASSERT_EQ(move.from, e4);
  ASSERT_EQ(move.to, c4);
}

TEST_F(SearchTestSuite, FindBestMove2) {
  // ApplyFen(position, "6k1/5pp1/4p3/1p1n2Qp/3P3P/BqP3P1/1P3PK1/8 w - - 0 1");

  // Move move = manager.FindBestMove(position);
  //
  // ASSERT_EQ(move.from, e4);
  // ASSERT_EQ(move.to, c4);
}
