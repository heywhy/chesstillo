#include <unordered_map>

#include <engine/engine.hpp>
#include <ftxui/component/component_base.hpp>
#include <ftxui/component/event.hpp>
#include <ftxui/dom/elements.hpp>

#include <tui/component/square.hpp>
#include <tui/fonts.hpp>
#include <tui/theme.hpp>

namespace tui {

// clang-format off
static tui::Square kSquareColor[64] = {
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

static std::unordered_map<char, std::pair<const char *, engine::Color>> kG = {
    {'r', {kRook, engine::BLACK}},   {'n', {kKnight, engine::BLACK}},
    {'b', {kBishop, engine::BLACK}}, {'k', {kKing, engine::BLACK}},
    {'q', {kQueen, engine::BLACK}},  {'p', {kPawn, engine::BLACK}},
    {'R', {kRook, engine::WHITE}},   {'N', {kKnight, engine::WHITE}},
    {'B', {kBishop, engine::WHITE}}, {'K', {kKing, engine::WHITE}},
    {'Q', {kQueen, engine::WHITE}},  {'P', {kPawn, engine::WHITE}},
};

Square::Square(const tui::Theme &theme, int index)
    : index(index), piece_('\0'), theme_(theme) {}

ftxui::Element component::Square::OnRender() {
  ftxui::Elements elements;

  if (piece_ != '\0') {
    auto [icon, color] = kG[piece_];

    elements.push_back(ftxui::text(icon) | ftxui::color(theme_.piece[color]));
  }

  ftxui::Element square = ftxui::vbox(elements) | ftxui::center |
                          ftxui::size(ftxui::WIDTH, ftxui::EQUAL, 7) |
                          ftxui::size(ftxui::HEIGHT, ftxui::EQUAL, 3);

  if (Focused()) {
    square |= ftxui::bgcolor(theme_.focused_square);
  } else {
    auto color = theme_.square[kSquareColor[index]];

    square |= ftxui::bgcolor(color);
  }

  return square | ftxui::reflect(box_);
}

bool Square::OnEvent(ftxui::Event event) {
  if (event.is_mouse()) {
    ftxui::Mouse &mouse = event.mouse();

    if (mouse.motion == ftxui::Mouse::Motion::Pressed &&
        mouse.button == ftxui::Mouse::Button::Left &&
        box_.Contain(mouse.x, mouse.y) && CaptureMouse(event)) {
      TakeFocus();

      return true;
    }
  }

  return ftxui::ComponentBase::OnEvent(event);
}

}  // namespace component
}  // namespace tui
