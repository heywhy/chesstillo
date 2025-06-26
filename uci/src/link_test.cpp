#include <sstream>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <uci/link.hpp>
#include <uci/process.hpp>

using namespace uci;

struct ProcessMock : public Process {
  MOCK_METHOD(void, Handle, (command::Input *), (override));
  MOCK_METHOD(void, Handle, (command::Debug *), (override));
  MOCK_METHOD(void, Handle, (command::SetOption *), (override));
  MOCK_METHOD(void, Handle, (command::Register *), (override));
  MOCK_METHOD(void, Handle, (command::Position *), (override));
  MOCK_METHOD(void, Handle, (command::Go *), (override));
};

class UCILinkTestSuite : public testing::Test {
 public:
  UCILinkTestSuite() : link(in, out, &process) {}

 protected:
  std::istringstream in;
  std::ostringstream out;
  testing::NiceMock<ProcessMock> process;
  Link link;

  void TearDown() override {
    in.clear();
    out.clear();
  }
};

TEST_F(UCILinkTestSuite, TestReceiveCommand) {
  in.str("uci\nisready\nucinewgame\nstop\nponderhit\nquit");

  EXPECT_CALL(process, Handle(testing::A<command::Input *>())).Times(5);

  link.Loop();
}

TEST_F(UCILinkTestSuite, TestSendCommand) {
  link.Send(command::Input("uciok"));

  ASSERT_EQ(out.str(), "uciok\n");
}
