#ifndef TUI_COMPONENT_INPUT_HPP
#define TUI_COMPONENT_INPUT_HPP

#include <string>
#include <string_view>

#include <ftxui/component/component.hpp>

namespace tui {
namespace component {

class Input : public ftxui::ComponentBase {
public:
  const std::string_view label;

  Input(const std::string_view label, std::string &value,
        bool multiline = false);

  ftxui::Element OnRender() override;

private:
  ftxui::Box box_;

  ftxui::Element Transform(ftxui::InputState state);
};

} // namespace component
} // namespace tui

#endif
