#include <functional>
#include <string>
#include <utility>

#include <ftxui/component/component.hpp>
#include <tui/component/input.hpp>

namespace tui {
namespace component {

Input::Input(std::string &value, bool multiline) {
  auto transform = std::bind(&Input::Transform, this, std::placeholders::_1);

  Add(ftxui::Input({.content = &value,
                    .transform = std::move(transform),
                    .multiline = multiline,
                    .cursor_position = value.length()}));
}

ftxui::Element Input::Transform(ftxui::InputState state) {

  state.element |= ftxui::color(ftxui::Color::White);

  if (state.focused) {
    state.element |=
        ftxui::borderStyled(ftxui::ROUNDED, ftxui::Color::DarkSlateGray1);
  } else {
    state.element |=
        ftxui::borderStyled(ftxui::ROUNDED, ftxui::Color::GrayDark);
  }

  return state.element;
}

} // namespace component
} // namespace tui
