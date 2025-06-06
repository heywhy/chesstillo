#include <ftxui/component/component.hpp>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <tui/component/modal.hpp>
#include <tui/utility.hpp>

using namespace tui;

class TUIComponentModalTestSuite : public testing::Test {
 public:
  TUIComponentModalTestSuite() : screen(10, 7) {}

 protected:
  ftxui::Screen screen;
  ftxui::Component main;
  ftxui::Component content;

  void SetUp() override {
    main = ftxui::Renderer([] { return ftxui::text("hello"); });
    content = ftxui::Renderer([] { return ftxui::text("world"); });
  }
};

TEST_F(TUIComponentModalTestSuite, TestDefaultRender) {
  auto modal = tui::Make<component::Modal>(main, content);

  ftxui::Render(screen, modal->Render());

  ASSERT_THAT(screen.ToString(), testing::HasSubstr("hello"));
  ASSERT_THAT(screen.ToString(), testing::Not(testing::HasSubstr("world")));
}

TEST_F(TUIComponentModalTestSuite, TestShowModal) {
  component::Modal modal(main, content);

  modal.Show();

  ftxui::Render(screen, modal.Render());

  ASSERT_THAT(screen.ToString(), testing::HasSubstr("world"));
}

TEST_F(TUIComponentModalTestSuite, TestHideModal) {
  component::Modal modal(main, content);

  modal.Show();

  ftxui::Render(screen, modal.Render());

  ASSERT_THAT(screen.ToString(), testing::HasSubstr("world"));

  modal.Hide();

  screen.Clear();

  ftxui::Render(screen, modal.Render());

  ASSERT_THAT(screen.ToString(), testing::Not(testing::HasSubstr("world")));
}

TEST_F(TUIComponentModalTestSuite, TestHideModalOnEsc) {
  auto modal = tui::Make<component::Modal>(main, content);

  modal->Show();
  modal->OnEvent(ftxui::Event::Escape);

  ftxui::Render(screen, modal->Render());

  ASSERT_THAT(screen.ToString(), testing::Not(testing::HasSubstr("world")));
}

TEST_F(TUIComponentModalTestSuite, TestHideModalOnQ) {
  auto modal = tui::Make<component::Modal>(main, content);

  modal->Show();
  modal->OnEvent(ftxui::Event::q);

  ftxui::Render(screen, modal->Render());

  ASSERT_THAT(screen.ToString(), testing::Not(testing::HasSubstr("world")));
}
