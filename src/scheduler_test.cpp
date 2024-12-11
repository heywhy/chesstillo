#include <chesstillo/scheduler.hpp>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

using namespace std::chrono_literals;
using testing::AtLeast;

class Job {
public:
  MOCK_METHOD(void, Invoke, ());
};

class SchedulerTestSuite : public testing::Test {
protected:
  Job job;
  Scheduler scheduler_;

  SchedulerTestSuite() : scheduler_(10) { scheduler_.Init(); }
};

TEST_F(SchedulerTestSuite, DispatchASimpleCallback) {
  EXPECT_CALL(job, Invoke()).Times(AtLeast(1));

  Status status = scheduler_.Dispatch([&]() { job.Invoke(); });

  status.Wait();
}

TEST_F(SchedulerTestSuite, DispatchASimpleCallback1) {
  EXPECT_CALL(job, Invoke()).Times(AtLeast(1));

  Status status = scheduler_.Dispatch([&]() {
    std::this_thread::sleep_for(1ms);
    job.Invoke();
  });

  ASSERT_EQ(scheduler_.Busy(), 1);

  status.Wait();

  ASSERT_EQ(scheduler_.Busy(), 0);
}
