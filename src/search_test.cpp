#include <chesstillo/fen.hpp>
#include <chesstillo/position.hpp>
#include <chesstillo/search.hpp>
#include <gtest/gtest.h>

class SearchTestSuite : public testing::Test {
protected:
  Position position;

  void SetUp() override { ApplyFen(position, START_FEN); }

  void TearDown() override { position.Reset(); }
};

TEST_F(SearchTestSuite, Dummy) {}
