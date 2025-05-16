#ifndef TUI_COMPONENT_INPUT_HPP
#define TUI_COMPONENT_INPUT_HPP

#include <string>

#include <ftxui/component/component.hpp>

namespace tui {
namespace component {

class Input : public ftxui::ComponentBase {
public:
  Input(std::string &value, bool multiline = false);

private:
  ftxui::Element Transform(ftxui::InputState state);
};

} // namespace component
} // namespace tui

#endif
