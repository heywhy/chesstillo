#include <cstddef>
#include <cstdio>
#include <vector>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <engine/scheduler.hpp>

using namespace engine;
using namespace std::chrono_literals;
using testing::AtLeast;

class Mock {
 public:
  MOCK_METHOD(void, Invoke, ());
};

class SchedulerTestSuite : public testing::Test {
 protected:
  Mock mock;
  Scheduler scheduler_;

  SchedulerTestSuite() : scheduler_(50) { scheduler_.Init(); }
};

TEST_F(SchedulerTestSuite, DispatchASimpleCallback) {
  EXPECT_CALL(mock, Invoke()).Times(AtLeast(1));

  Status *status = scheduler_.Dispatch([&]() { mock.Invoke(); });

  status->Wait();
  delete status;
}

TEST_F(SchedulerTestSuite, DispatchASimpleCallback1) {
  EXPECT_CALL(mock, Invoke()).Times(AtLeast(1));

  Status *status = scheduler_.Dispatch([&]() {
    std::this_thread::sleep_for(1ms);
    mock.Invoke();
  });

  ASSERT_EQ(scheduler_.Busy(), 1);

  status->Wait();
  delete status;

  ASSERT_EQ(scheduler_.Busy(), 0);
}

TEST_F(SchedulerTestSuite, OutOfWorkers) {
  std::vector<Status *> jobs;
  size_t size = scheduler_.Size();
  size += size / 2;

  jobs.reserve(size);

  EXPECT_CALL(mock, Invoke()).Times(AtLeast(size + 1));

  for (size_t i = 0; i < size; i++) {
    Status *s = scheduler_.Dispatch([this] { mock.Invoke(); });

    jobs.push_back(s);
  }

  Status *status = scheduler_.Dispatch([this] { mock.Invoke(); });

  status->Wait();

  for (Status *status : jobs) {
    status->Wait();
    delete status;
  }

  delete status;
}
