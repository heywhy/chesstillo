#include <gtest/gtest.h>

#include <engine/search.hpp>
#include <engine/types.hpp>

using namespace engine;

class WorkerTestSuite : public testing::Test {
 protected:
  Search search;
  search::WorkerRegistry registry;

  WorkerTestSuite() : registry(2), search(&registry) {}
};

TEST_F(WorkerTestSuite, TestGetIdleWorker) {
  search::Worker *worker = registry.GetIdleWorker();

  ASSERT_NE(worker, nullptr);
  ASSERT_EQ(registry.IdleWorkers(), 1);
}

TEST_F(WorkerTestSuite, TestPutIdleWorker) {
  search::Worker *worker = registry.GetIdleWorker();

  ASSERT_EQ(registry.IdleWorkers(), 1);

  registry.PutIdleWorker(worker);

  ASSERT_EQ(registry.IdleWorkers(), 2);
}

TEST_F(WorkerTestSuite, TestAssignNodeToWorker) {
  search::Node node(&search, MIN_SCORE, MAX_SCORE, 2);
  search::Worker *worker = registry.GetIdleWorker();

  ASSERT_EQ(registry.IdleWorkers(), 1);

  worker->Assign(&node);

  while (registry.IdleWorkers() < 2) {
  }
}
