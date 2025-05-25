// #include <string>
//
// #include <ftxui/component/component.hpp>
// #include <gtest/gtest.h>
// #include <tui/component/input.hpp>
//
// using namespace tui;
//
// TEST(TUIComponentInput, Render) {
//   std::string content("hello world");
//   ftxui::Component input = ftxui::Make<component::Input>("", content);
//
//   auto document = input->Render();
//   auto screen = ftxui::Screen::Create(ftxui::Dimension::Fit(document));
//
//   ftxui::Render(screen, document);
//
//   ASSERT_EQ(screen.ToString(),
//             " ╭────────────╮\r\n"
//             " │hello world │\r\n"
//             " ╰────────────╯");
// }
