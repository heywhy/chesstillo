#include <ftxui/component/component.hpp>
#include <ftxui/component/event.hpp>
#include <gtest/gtest.h>

#include <tui/component/modal_view.hpp>
#include <tui/mapping.hpp>

using namespace tui;

class TUIComponentModalViewTestSuite : public testing::Test {
 protected:
  component::ModalView view;

  void SetUp() override { GTEST_FLAG_SET(death_test_style, "threadsafe"); }
};

TEST_F(TUIComponentModalViewTestSuite, TestSwitchToInteractMode) {
  ASSERT_EQ(view.Mode(), tui::NORMAL);

  view.OnEvent(ftxui::Event::i);

  ASSERT_EQ(view.Mode(), tui::INTERACT);
}

TEST_F(TUIComponentModalViewTestSuite, TestSwitchToVisualMode) {
  view.OnEvent(ftxui::Event::v);

  ASSERT_EQ(view.Mode(), tui::VISUAL);

  view.OnEvent(ftxui::Event::Escape);

  ASSERT_EQ(view.Mode(), tui::NORMAL);
}

TEST_F(TUIComponentModalViewTestSuite, TestHandleNormalModeKeymap) {
  int count = 0;
  auto fun = [&] { count++; };

  view.SetKeymap(tui::NORMAL, "e", fun);
  view.SetKeymap(tui::NORMAL, "<c-a>", fun);

  view.OnEvent(ftxui::Event::e);
  view.OnEvent(ftxui::Event::CtrlA);
  view.OnEvent(ftxui::Event::a);
  view.OnEvent(ftxui::Event::e);

  ASSERT_EQ(count, 2);
}

TEST_F(TUIComponentModalViewTestSuite, TestHandleVisualModeKeymap) {
  int count = 0;
  auto fun = [&] { count++; };
  component::ModalView view(tui::VISUAL);

  view.SetKeymap(tui::VISUAL, "f", fun);
  view.SetKeymap(tui::VISUAL, "<s-d>", fun);

  view.OnEvent(ftxui::Event::f);
  view.OnEvent(ftxui::Event::D);

  ASSERT_EQ(count, 2);
}

TEST_F(TUIComponentModalViewTestSuite, TestPassEventsToNestedModalView) {
  int child_keymap_called = 0;
  auto child = ftxui::Make<component::ModalView>();

  child->SetKeymap(tui::NORMAL, "e", [&] { child_keymap_called++; });

  view.Add(child);

  child->FocusView();

  view.OnEvent(ftxui::Event::e);

  child->UnfocusView();

  view.OnEvent(ftxui::Event::e);

  ASSERT_EQ(child_keymap_called, 1);
}
