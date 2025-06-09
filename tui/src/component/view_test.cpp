#include <ftxui/component/component.hpp>
#include <ftxui/component/event.hpp>
#include <ftxui/dom/node.hpp>
#include <ftxui/screen/screen.hpp>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <tui/component/modal_view.hpp>
#include <tui/component/view.hpp>
#include <tui/mapping.hpp>
#include <tui/utility.hpp>

using namespace tui;

class Child : public component::ModalView {
 public:
  Child() : component::ModalView(tui::NORMAL) {}

  void BindKeymaps() override {}
};

TEST(TUIComponentViewTestSuite, TestSwitchToInteractMode) {
  component::View view;

  ASSERT_EQ(view.Mode(), tui::NORMAL);

  view.OnEvent(ftxui::Event::i);

  ASSERT_EQ(view.Mode(), tui::INTERACT);
}

TEST(TUIComponentViewTestSuite, TestSwitchToVisualMode) {
  component::View view;

  view.OnEvent(ftxui::Event::v);

  ASSERT_EQ(view.Mode(), tui::VISUAL);

  view.OnEvent(ftxui::Event::Escape);

  ASSERT_EQ(view.Mode(), tui::NORMAL);
}

TEST(TUIComponentViewTestSuite, TestHandleNormalModeKeymap) {
  component::View view;

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

TEST(TUIComponentViewTestSuite, TestHandleVisualModeKeymap) {
  int count = 0;
  auto fun = [&] { count++; };
  component::View view(tui::VISUAL);

  view.SetKeymap(tui::VISUAL, "f", fun);
  view.SetKeymap(tui::VISUAL, "<s-d>", fun);

  view.OnEvent(ftxui::Event::f);
  view.OnEvent(ftxui::Event::D);

  ASSERT_EQ(count, 2);
}

TEST(TUIComponentViewTestSuite, TestCommandPrompt) {
  component::View view;
  ftxui::Screen screen(10, 10);

  RENDER(screen, view.Render());

  ASSERT_THAT(screen.ToString(), testing::Not(testing::HasSubstr("Cmdline")));

  view.OnEvent(ftxui::Event::Character(":"));

  RENDER(screen, view.Render());

  ASSERT_THAT(screen.ToString(), testing::HasSubstr("Cmdline"));

  view.OnEvent(ftxui::Event::Escape);

  RENDER(screen, view.Render());

  ASSERT_THAT(screen.ToString(), testing::Not(testing::HasSubstr("Cmdline")));
}

TEST(TUIComponentViewTestSuite, TestSendEventToCommandPromptWhenIsVisible) {
  component::View view;
  ftxui::Screen screen(10, 10);

  view.OnEvent(ftxui::Event::Character(":"));
  view.OnEvent(ftxui::Event::e);
  view.OnEvent(ftxui::Event::d);
  view.OnEvent(ftxui::Event::i);
  view.OnEvent(ftxui::Event::t);

  RENDER(screen, view.Render());

  ASSERT_THAT(screen.ToString(), testing::HasSubstr("edit"));
}

TEST(TUIComponentViewTestSuite, TestPassEventsToNestedModalView) {
  component::View view;

  int child_keymap_called = 0;
  auto child = tui::Make<Child>();

  child->SetKeymap(tui::NORMAL, "e", [&] { child_keymap_called++; });

  view.Add(child);

  child->TakeFocus();

  ASSERT_EQ(child.get(), view.ActiveChild().get());

  view.OnEvent(ftxui::Event::e);

  child->Detach();

  ASSERT_NE(child.get(), view.ActiveChild().get());

  view.OnEvent(ftxui::Event::e);

  ASSERT_EQ(child_keymap_called, 1);
}

TEST(TUIComponentViewTestSuite, TestModal) {
  component::View view;
  ftxui::Screen screen(20, 10);
  auto main = ftxui::Renderer([] { return ftxui::text("main content"); });
  auto modal = ftxui::Renderer([] { return ftxui::text("modal content"); });

  view.Add(main);

  main->TakeFocus();

  view.ShowModal(modal);

  RENDER(screen, view.Render());

  ASSERT_EQ(view.ActiveChild().get(), modal.get());
  ASSERT_THAT(screen.ToString(), testing::HasSubstr("modal content"));

  view.OnEvent(ftxui::Event::Escape);

  RENDER(screen, view.Render());

  ASSERT_EQ(view.ActiveChild().get(), main.get());
  ASSERT_THAT(screen.ToString(),
              testing::Not(testing::HasSubstr("modal content")));
}
