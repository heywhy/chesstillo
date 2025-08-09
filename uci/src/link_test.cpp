#include <istream>
#include <ostream>
#include <sstream>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "uci/link.hpp"

using namespace uci;

struct ProcessMock : public Link {
  ProcessMock(std::istream &in, std::ostream &out) : Link(in, out) {}

  MOCK_METHOD(void, Handle, (command::Input *), (override));
  MOCK_METHOD(void, Handle, (command::Debug *), (override));
  MOCK_METHOD(void, Handle, (command::SetOption *), (override));
  MOCK_METHOD(void, Handle, (command::Register *), (override));
  MOCK_METHOD(void, Handle, (command::Position *), (override));
  MOCK_METHOD(void, Handle, (command::Go *), (override));
};

class UCILinkTestSuite : public testing::Test {
 public:
  UCILinkTestSuite() : link(in, out) {}

 protected:
  std::istringstream in;
  std::ostringstream out;
  testing::NiceMock<ProcessMock> link;

  void TearDown() override {
    in.clear();
    out.clear();
  }
};

TEST_F(UCILinkTestSuite, TestReceiveCommand) {
  in.str("uci\nisready\nucinewgame\nstop\nponderhit\nquit");

  EXPECT_CALL(link, Handle(testing::A<command::Input *>())).Times(5);

  link.Loop();
}

TEST_F(UCILinkTestSuite, TestSendCommand) {
  link.Send(command::Input("uciok"));

  ASSERT_EQ(out.str(), "uciok\n");
}

TEST_F(UCILinkTestSuite, TestHandleUnknownCommand) {
  in.str("uciok\nquit");

  EXPECT_CALL(link, Handle(testing::A<command::Input *>())).Times(0);

  link.Loop();

  ASSERT_EQ(out.str(), "Error at 'uciok': Unknown command.\n");
}
