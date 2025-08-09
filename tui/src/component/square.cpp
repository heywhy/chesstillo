#include <utility>

#include <ftxui/component/component_base.hpp>
#include <ftxui/component/event.hpp>
#include <ftxui/dom/elements.hpp>

#include "tui/component/square.hpp"
#include "tui/constants.hpp"
#include "tui/theme.hpp"

namespace tui {

// clang-format off
static const tui::Square kSquareColor[64] = {
  DARK, LIGHT, DARK, LIGHT, DARK, LIGHT, DARK, LIGHT,
  LIGHT, DARK, LIGHT, DARK, LIGHT, DARK, LIGHT, DARK,
  DARK, LIGHT, DARK, LIGHT, DARK, LIGHT, DARK, LIGHT,
  LIGHT, DARK, LIGHT, DARK, LIGHT, DARK, LIGHT, DARK,
  DARK, LIGHT, DARK, LIGHT, DARK, LIGHT, DARK, LIGHT,
  LIGHT, DARK, LIGHT, DARK, LIGHT, DARK, LIGHT, DARK,
  DARK, LIGHT, DARK, LIGHT, DARK, LIGHT, DARK, LIGHT,
  LIGHT, DARK, LIGHT, DARK, LIGHT, DARK, LIGHT, DARK,
};
// clang-format on

namespace component {

Square::Square(const tui::Theme &theme, int index, OnClick on_click)
    : index(index),
      piece_('\0'),
      hover_(false),
      selected_(false),
      theme_(theme),
      on_click_(std::move(on_click)) {}

ftxui::Element component::Square::OnRender() {
  ftxui::Elements elements;

  if (piece_ != '\0') {
    auto [icon, color] = kPieceFontsMap[piece_];

    elements.push_back(ftxui::text(icon) | ftxui::color(theme_.piece[color]));
  }

  ftxui::Element square = ftxui::dbox(elements) | ftxui::center |
                          ftxui::size(ftxui::WIDTH, ftxui::EQUAL, 7) |
                          ftxui::size(ftxui::HEIGHT, ftxui::EQUAL, 3);

  auto state = kSquareColor[index];

  if (selected_) {
    state = SELECTED;
  } else if (Focused()) {
    state = FOCUSED;
  } else if (hover_) {
    state = HOVER;
  }

  square |= ftxui::bgcolor(theme_.square[state]);

  return square | ftxui::reflect(box_);
}

bool Square::OnEvent(ftxui::Event event) {
  hover_ = false;

  if (event.is_mouse()) {
    ftxui::Mouse &mouse = event.mouse();

    if ((mouse.motion == ftxui::Mouse::Motion::Pressed ||
         mouse.motion == ftxui::Mouse::Motion::Released) &&
        mouse.button == ftxui::Mouse::Button::Left &&
        box_.Contain(mouse.x, mouse.y) && CaptureMouse(event)) {
      TakeFocus();
      on_click_(this);

      return true;
    }

    if (mouse.motion == ftxui::Mouse::Motion::Moved &&
        mouse.button == ftxui::Mouse::Button::Left &&
        box_.Contain(mouse.x, mouse.y) && CaptureMouse(event)) {
      hover_ = true;
    }
  }

  return ftxui::ComponentBase::OnEvent(event);
}

}  // namespace component
}  // namespace tui
