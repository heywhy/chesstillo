#include <thread>

#include <chesstillo/options.hpp>
#include <chesstillo/search.hpp>
#include <chesstillo/types.hpp>
#include <chesstillo/ybwc.hpp>
#include <gtest/gtest.h>

using namespace std::chrono_literals;

class YBWCTestSuite : public testing::Test {
protected:
  Search *search;

  void SetUp() override {
    GTEST_FLAG_SET(death_test_style, "threadsafe");

    options.tasks = std::thread::hardware_concurrency();

    search = new Search(options.tasks);

    search->height = 0;
  }

  void TearDown() override { delete search; }
};

TEST_F(YBWCTestSuite, Task) { Task task; };

TEST_F(YBWCTestSuite, InitTaskStack) {
  TaskStack stack(1);
  Task *task = stack.GetIdleTask();

  ASSERT_NE(task, nullptr);
};

TEST_F(YBWCTestSuite, InitializeNode) {
  Node node(search, 100, 500, 4);

  ASSERT_EQ(node.alpha, 100);
  ASSERT_EQ(node.beta, 500);
  ASSERT_EQ(node.depth, 4);

  ASSERT_DEATH(Node(search, 100, 50, 5), "Assertion failed");
};

TEST_F(YBWCTestSuite, Split) {
  Move move(e2, e4, PAWN);
  Node node(search, -SCORE_INF, SCORE_INF, 6);

  ASSERT_FALSE(node.Split(move));

  search->stop = RUNNING;

  node.moves_done = 1;
  node.moves_todo = 2;

  ASSERT_TRUE(node.Split(move));

  node.WaitSlaves();
};
