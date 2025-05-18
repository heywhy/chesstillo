#include <ftxui/component/event.hpp>
#include <ftxui/screen/color.hpp>
#include <tui/component/square.hpp>
#include <tui/theme.hpp>

// TODO: temporary hack, use the constants from chesstillo
// clang-format off
static bool dark[64] = {
  true, false, true, false, true, false, true, false,
  false, true, false, true, false, true, false, true,
  true, false, true, false, true, false, true, false,
  false, true, false, true, false, true, false, true,
  true, false, true, false, true, false, true, false,
  false, true, false, true, false, true, false, true,
  true, false, true, false, true, false, true, false,
  false, true, false, true, false, true, false, true,
};
// clang-format on

namespace tui {
namespace component {

Square::Square(const tui::Theme &theme, int index)
    : index(index), theme_(theme) {}

ftxui::Element component::Square::OnRender() {
  ftxui::Elements elements;
  ftxui::Element square = ftxui::vbox(elements) | ftxui::center |
                          ftxui::size(ftxui::WIDTH, ftxui::EQUAL, 7) |
                          ftxui::size(ftxui::HEIGHT, ftxui::EQUAL, 3);

  if (Focused()) {
    square |= ftxui::bgcolor(theme_.focused_square);
  } else {
    ftxui::Color color = dark[index] ? theme_.dark_square : theme_.light_square;

    square |= ftxui::bgcolor(color);
  }

  return square | ftxui::reflect(box_);
}

bool Square::OnEvent(ftxui::Event event) {
  if (event.is_mouse()) {
    ftxui::Mouse &mouse = event.mouse();

    if (mouse.motion == ftxui::Mouse::Motion::Pressed &&
        mouse.button == ftxui::Mouse::Button::Left &&
        box_.Contain(mouse.x, mouse.y)) {
      Parent()->SetActiveChild(this);

      return true;
    }
  }

  return ftxui::ComponentBase::OnEvent(event);
}

} // namespace component
} // namespace tui
