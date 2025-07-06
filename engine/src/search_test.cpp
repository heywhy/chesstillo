#include <thread>

#include <gtest/gtest.h>

#include <engine/position.hpp>
#include <engine/search.hpp>
#include <engine/transposition.hpp>

using namespace engine;

static const auto kHardwareThreads = std::thread::hardware_concurrency();

class SearchTestSuite : public testing::Test {
 protected:
  TT tt;
  Search search;
  Position position;
  search::WorkerRegistry workers;

  SearchTestSuite()
      : tt(MAX_TRANSPOSITION_SIZE),
        workers(kHardwareThreads),
        search(&workers) {}

  void SetUp() override {
    Position::ApplyFen(&position, kStartPos);

    search.position = &position;
    search.tt = &tt;
  }

  void TearDown() override { position.Reset(); }
};

TEST_F(SearchTestSuite, TestAssignNodeToWorker) {
  // mm
  search.Run();
}
