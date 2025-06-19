#include <chesstillo/fen.hpp>
#include <chesstillo/position.hpp>
#include <chesstillo/types.hpp>
#include <gtest/gtest.h>

class MoveGenTestSuite : public testing::Test {
protected:
  Position position;

  void SetUp() override { ApplyFen(position, START_FEN); }

  void TearDown() override { position.Reset(); }
};
