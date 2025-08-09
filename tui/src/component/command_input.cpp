#include <string>

#include <ftxui/component/component.hpp>

#include "tui/component/command_input.hpp"

namespace tui {
namespace component {

CommandInput::CommandInput(std::string &value) {
  Add(ftxui::Input({.content = &value,
                    .transform = &CommandInput::Transform,
                    .multiline = false,
                    .cursor_position = value.length()}));
}

ftxui::Element CommandInput::OnRender() {
  ftxui::Element input =
      ChildAt(0)->Render() | ftxui::vcenter | ftxui::xflex_grow;

  return ftxui::dbox({input, ftxui::text(" Cmdline ") | ftxui::hcenter});
}

ftxui::Element CommandInput::Transform(ftxui::InputState state) {
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
