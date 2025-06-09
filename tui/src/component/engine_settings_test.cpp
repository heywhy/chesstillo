#include <ftxui/component/component.hpp>
#include <gtest/gtest.h>

#include <tui/component/engine_settings.hpp>
#include <tui/config.hpp>
#include <tui/utility.hpp>

using namespace tui;

TEST(TUIComponentEngineSettingsTestSuite, RenderCheck) {
  EngineOption option;

  option.value = true;

  ftxui::Component check =
      tui::Make<component::EngineSettings::Check>("Ponder", option);

  auto document = check->Render();
  auto screen = ftxui::Screen::Create(ftxui::Dimension::Fit(document));

  ftxui::Render(screen, document);

  ASSERT_EQ(screen.ToString(), "▣ \x1B[1m\x1B[7mPonder\x1B[22m\x1B[27m");
}

TEST(TUIComponentEngineSettingsTestSuite, RenderSpin) {
  EngineOption option;
  std::string_view label("Threads");

  option.value = 1;
  option.min = 1;
  option.max = 1024;

  ftxui::Component spin =
      tui::Make<component::EngineSettings::Spin>(label, option);

  auto document = spin->Render();
  auto screen = ftxui::Screen::Create(ftxui::Dimension::Fit(document));

  ftxui::Render(screen, document);

  ASSERT_EQ(screen.ToString(),
            "Threads [\x1B[4m\x1B[97m\x1B[49m1\x1B[24m\x1B[39m\x1B[49m]");
}
