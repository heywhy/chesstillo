#include <gtest/gtest.h>

#include <engine/move.hpp>
#include <engine/position.hpp>
#include <engine/transposition.hpp>
#include <engine/types.hpp>
#include <engine/utility.hpp>

using namespace engine;

class TranspositionTestSuite : public testing::Test {
 public:
  TranspositionTestSuite() : tt(MAX_TRANSPOSITION_SIZE) {};

 protected:
  Position position;
  TT tt;

  void SetUp() override { Position::ApplyFen(&position, kStartPos); }

  void TearDown() override {
    tt.Clear();
    position.Reset();
  }
};

TEST_F(TranspositionTestSuite, TestAddEntry) {
  TTEntry entry;
  Move move = DeduceMove(position, e2, e4);

  ASSERT_FALSE(tt.Probe(position, &entry));

  tt.Add(position, 2, 100, move, NodeType::PV);

  ASSERT_TRUE(tt.Probe(position, &entry));
  ASSERT_EQ(entry.depth, 2);
  ASSERT_EQ(entry.score, 100);
  ASSERT_EQ(entry.best_move, move);
  ASSERT_EQ(entry.node, NodeType::PV);
  ASSERT_EQ(entry.age, 0);
}

TEST_F(TranspositionTestSuite, TestIgnoreEntryBasedOnDepth) {
  TTEntry entry;
  Move move = DeduceMove(position, e2, e4);

  tt.Add(position, 8, 100, move, NodeType::ALL);

  tt.Add(position, 2, 100, move, NodeType::CUT);

  ASSERT_TRUE(tt.Probe(position, &entry));
  ASSERT_EQ(entry.depth, 8);
}

TEST_F(TranspositionTestSuite, TestAgedEntryIsOverwritten) {
  TTEntry entry;
  Move move = DeduceMove(position, e2, e4);

  Position::ApplyFen(&position, "8/8/3N4/2r5/1p2RN2/1rk5/2p5/2K5 w - - 0 1");

  tt.Add(position, 2, 100, move, NodeType::ALL);

  Position::ApplyFen(&position, "8/8/3N4/2r5/1p2RN2/1rk5/2p5/2K5 w - - 5 1");

  tt.Add(position, 6, -75, move, NodeType::ALL);

  ASSERT_TRUE(tt.Probe(position, &entry));
  ASSERT_EQ(entry.depth, 6);
  ASSERT_EQ(entry.score, -75);
}

TEST_F(TranspositionTestSuite, TestApplyPVCutOff) {
  int score;
  Move best_move;
  Move move = DeduceMove(position, e2, e4);

  tt.Add(position, 4, 100, move, NodeType::PV);

  ASSERT_TRUE(tt.CutOff(position, 1, 10, 10, &best_move, &score));
  ASSERT_EQ(best_move, move);
  ASSERT_EQ(score, 100);
}

TEST_F(TranspositionTestSuite, TestApplyAlphaCutOff) {
  int score;
  Move best_move;
  Move move = DeduceMove(position, g2, e4);

  tt.Add(position, 5, -25, move, NodeType::ALL);

  ASSERT_TRUE(tt.CutOff(position, 2, -10, 20, &best_move, &score));
  ASSERT_EQ(best_move, move);
  ASSERT_EQ(score, -25);
  ASSERT_FALSE(tt.CutOff(position, 10, -10, 20, &best_move, &score));
}

TEST_F(TranspositionTestSuite, TestApplyBetaCutOff) {
  int score;
  Move best_move;
  Move move = DeduceMove(position, g2, e4);

  tt.Add(position, 5, 25, move, NodeType::CUT);

  ASSERT_TRUE(tt.CutOff(position, 2, -50, 20, &best_move, &score));
  ASSERT_EQ(best_move, move);
  ASSERT_EQ(score, 25);
  ASSERT_FALSE(tt.CutOff(position, 8, -50, 20, &best_move, &score));
}
