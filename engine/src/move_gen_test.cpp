#include <gtest/gtest.h>

#include <engine/position.hpp>
#include <engine/types.hpp>

using namespace engine;

class MoveGenTestSuite : public testing::Test {
 protected:
  Position position;

  void SetUp() override { position = Position::FromFen(kStartPos); }

  void TearDown() override { position.Reset(); }
};
