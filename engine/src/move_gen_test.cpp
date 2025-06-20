#include <gtest/gtest.h>

#include <engine/fen.hpp>
#include <engine/position.hpp>
#include <engine/types.hpp>

using namespace engine;

class MoveGenTestSuite : public testing::Test {
 protected:
  Position position;

  void SetUp() override { ApplyFen(position, START_FEN); }

  void TearDown() override { position.Reset(); }
};
