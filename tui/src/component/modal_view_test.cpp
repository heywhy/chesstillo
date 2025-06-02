#include <ftxui/component/component.hpp>
#include <ftxui/component/event.hpp>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <tui/component/modal_view.hpp>

using namespace tui;

class Mock {
 public:
  MOCK_METHOD(void, Call, ());
};

class TUIComponentModalViewTestSuite : public testing::Test {
 protected:
  Mock mock;
  component::ModalView view;

  void SetUp() override { GTEST_FLAG_SET(death_test_style, "threadsafe"); }
};

TEST_F(TUIComponentModalViewTestSuite, TestSwitchToInteractMode) {
  ASSERT_EQ(view.Mode(), tui::NORMAL);

  view.OnEvent(ftxui::Event::i);

  ASSERT_EQ(view.Mode(), tui::INTERACT);
}

TEST_F(TUIComponentModalViewTestSuite, TestSwitchToVisualMode) {
  component::ModalView view(tui::INTERACT);

  ASSERT_EQ(view.Mode(), tui::INTERACT);

  view.OnEvent(ftxui::Event::Escape);

  ASSERT_EQ(view.Mode(), tui::NORMAL);
}

TEST_F(TUIComponentModalViewTestSuite, TestBindKeymaps) {
  view.SetKeymap(tui::NORMAL, "<esc>", "<nop>");
  view.SetKeymap(tui::NORMAL, "<c-a>", "<nop>");

  auto keymaps = view.GetKeymaps(tui::NORMAL);

  ASSERT_EQ(keymaps.size(), 3);

  ASSERT_EQ(keymaps[0]->orig_lhs, "<C-A>");
  ASSERT_TRUE(keymaps[0]->rhs_is_noop);

  ASSERT_EQ(keymaps[1]->orig_lhs, "<ESC>");
  ASSERT_TRUE(keymaps[1]->rhs_is_noop);
}

TEST_F(TUIComponentModalViewTestSuite, TestApplyKeymap) {
  auto fun = [&] { mock.Call(); };

  EXPECT_CALL(mock, Call()).Times(2);

  view.SetKeymap(tui::NORMAL, "e", fun);
  view.SetKeymap(tui::NORMAL, "<c-a>", fun);

  view.OnEvent(ftxui::Event::e);
  view.OnEvent(ftxui::Event::CtrlA);
  view.OnEvent(ftxui::Event::a);
  view.OnEvent(ftxui::Event::e);
}

TEST_F(TUIComponentModalViewTestSuite, TestApplyKeymapInVisualMode) {
  auto fun = [&] { mock.Call(); };
  component::ModalView view(tui::VISUAL);

  EXPECT_CALL(mock, Call()).Times(2);

  view.SetKeymap(tui::VISUAL, "f", fun);
  view.SetKeymap(tui::VISUAL, "<s-d>", fun);

  view.OnEvent(ftxui::Event::f);
  view.OnEvent(ftxui::Event::D);
}
