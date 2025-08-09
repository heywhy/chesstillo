#include <functional>
#include <string>
#include <string_view>
#include <utility>

#include <ftxui/component/component.hpp>

#include "tui/component/input.hpp"

namespace tui {
namespace component {

Input::Input(const std::string_view label, std::string &value,
             InputOption options)
    : Input(label, value, false, options) {}

Input::Input(const std::string_view label, std::string &value, bool multiline)
    : Input(label, value, multiline, {}) {}

Input::Input(const std::string_view label, std::string &value, bool multiline,
             InputOption options)
    : label(label) {
  auto transform = std::bind(&Input::Transform, this, std::placeholders::_1);

  Add(ftxui::Input({
      .content = &value,
      .transform = std::move(transform),
      .multiline = multiline,
      .on_change = std::move(options.on_change),
      .on_enter = std::move(options.on_enter),
      .cursor_position = value.length(),
  }));
}

ftxui::Element Input::OnRender() {
  ftxui::Element input =
      ChildAt(0)->Render() | ftxui::vcenter | ftxui::xflex_grow;

  return ftxui::hbox({ftxui::text(label.data()) | ftxui::vcenter,
                      ftxui::separatorEmpty(), input});
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

}  // namespace component
}  // namespace tui
