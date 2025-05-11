#include <ftxui/screen/color.hpp>
#include <tui/theme.hpp>

using namespace tui;

const Theme Theme::Default = {
    ftxui::Color::Grey93, ftxui::Color::Black,
    ftxui::Color(ftxui::Color::Palette256::DarkGoldenrod), ftxui::Color::White,
    ftxui::Color::Green};
