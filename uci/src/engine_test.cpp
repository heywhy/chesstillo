#include <condition_variable>
#include <mutex>
#include <string>
#include <string_view>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "uci/command.hpp"
#include "uci/engine.hpp"
#include "uci/ui.hpp"
#include "uci/utils.hpp"

using namespace std::chrono_literals;
using namespace testing;
using namespace uci;

struct UIMock : public UI {
  std::mutex mutex;
  std::condition_variable cv;

  void Notify(Command *) { cv.notify_all(); }

  MOCK_METHOD(void, Handle, (command::Input *), (override));
  MOCK_METHOD(void, Handle, (command::ID *), (override));
  MOCK_METHOD(void, Handle, (command::BestMove *), (override));
  MOCK_METHOD(void, Handle, (command::CopyProtection *), (override));
  MOCK_METHOD(void, Handle, (command::Registration *), (override));
  MOCK_METHOD(void, Handle, (command::Info *), (override));
  MOCK_METHOD(void, Handle, (command::Option *), (override));
};

static const std::string path_to_exe = FindExecutable("stockfish");

class UCIEngineTestSuite : public Test {
 protected:
  testing::NiceMock<UIMock> ui;
  Engine engine{path_to_exe, &ui};
};

TEST_F(UCIEngineTestSuite, TestSendCommand) {
  std::unique_lock lock(ui.mutex);

  ON_CALL(ui, Handle(testing::A<command::Input *>()))
      .WillByDefault([&](command::Input *command) {
        if (command->type == TokenType::UCI_OK) {
          ui.cv.notify_all();
        }
      });

  EXPECT_CALL(ui, Handle(A<command::Input *>())).Times(1);

  engine.Send(command::Input("uci"));

  ui.cv.wait_for(lock, 1000ms);
}
