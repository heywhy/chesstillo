#include <chesstillo/fen.hpp>
#include <chesstillo/transposition.hpp>
#include <chesstillo/types.hpp>
#include <chesstillo/utility.hpp>
#include <gtest/gtest.h>

class TranspositionTestSuite : public testing::Test {
public:
  TranspositionTestSuite() : tt(MAX_TRANSPOSITION_SIZE) {};

protected:
  Position position;
  TT tt;

  void SetUp() override { ApplyFen(position, START_FEN); }

  void TearDown() override {
    tt.Clear();
    position.Reset();
  }
};

TEST_F(TranspositionTestSuite, AddEntry) {
  TTEntry entry;
  Move move = DeduceMove(position, e2, e4);

  ASSERT_FALSE(tt.Probe(position, &entry));

  tt.Add(position, 2, 100, move, PV);

  ASSERT_TRUE(tt.Probe(position, &entry));
  ASSERT_EQ(entry.depth, 2);
  ASSERT_EQ(entry.score, 100);
  ASSERT_EQ(entry.best_move, move);
  ASSERT_EQ(entry.node, PV);
  ASSERT_EQ(entry.age, 0);
}

TEST_F(TranspositionTestSuite, IgnoreEntryBasedOnDepth) {
  TTEntry entry;
  Move move = DeduceMove(position, e2, e4);

  tt.Add(position, 8, 100, move, ALL);

  tt.Add(position, 2, 100, move, CUT);

  ASSERT_TRUE(tt.Probe(position, &entry));
  ASSERT_EQ(entry.depth, 8);
}

TEST_F(TranspositionTestSuite, AgedEntryIsOverwritten) {
  TTEntry entry;
  Move move = DeduceMove(position, e2, e4);

  ApplyFen(position, "8/8/3N4/2r5/1p2RN2/1rk5/2p5/2K5 w - - 0 1");

  tt.Add(position, 2, 100, move, ALL);

  ApplyFen(position, "8/8/3N4/2r5/1p2RN2/1rk5/2p5/2K5 w - - 5 1");

  tt.Add(position, 6, -75, move, ALL);

  ASSERT_TRUE(tt.Probe(position, &entry));
  ASSERT_EQ(entry.depth, 6);
  ASSERT_EQ(entry.score, -75);
}

TEST_F(TranspositionTestSuite, ApplyPVCutOff) {
  int score;
  Move best_move;
  Move move = DeduceMove(position, e2, e4);

  tt.Add(position, 4, 100, move, PV);

  ASSERT_TRUE(tt.CutOff(position, 1, 10, 10, &best_move, &score));
  ASSERT_EQ(best_move, move);
  ASSERT_EQ(score, 100);
}

TEST_F(TranspositionTestSuite, ApplyAlphaCutOff) {
  int score;
  Move best_move;
  Move move = DeduceMove(position, g2, e4);

  tt.Add(position, 5, -25, move, ALL);

  ASSERT_TRUE(tt.CutOff(position, 2, -10, 20, &best_move, &score));
  ASSERT_EQ(best_move, move);
  ASSERT_EQ(score, -25);
  ASSERT_FALSE(tt.CutOff(position, 10, -10, 20, &best_move, &score));
}

TEST_F(TranspositionTestSuite, ApplyBetaCutOff) {
  int score;
  Move best_move;
  Move move = DeduceMove(position, g2, e4);

  tt.Add(position, 5, 25, move, CUT);

  ASSERT_TRUE(tt.CutOff(position, 2, -50, 20, &best_move, &score));
  ASSERT_EQ(best_move, move);
  ASSERT_EQ(score, 25);
  ASSERT_FALSE(tt.CutOff(position, 8, -50, 20, &best_move, &score));
}
