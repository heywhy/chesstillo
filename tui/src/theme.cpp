#include <ftxui/screen/color.hpp>

#include <tui/theme.hpp>

namespace tui {

const Theme Theme::Default = {
    {ftxui::Color::Grey93, ftxui::Color::Black},
    {
        ftxui::Color::White,
        ftxui::Color::DarkGoldenrod,
        ftxui::Color::Green,
        ftxui::Color::DarkSlateGray1,
        ftxui::Color::Gold1,
    },
};

}
