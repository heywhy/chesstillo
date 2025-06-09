#ifndef TUI_COMPONENT_COMMAND_INPUT_HPP
#define TUI_COMPONENT_COMMAND_INPUT_HPP

#include <string>

#include <ftxui/component/component_base.hpp>
#include <ftxui/component/component_options.hpp>

namespace tui {
namespace component {

class CommandInput : public ftxui::ComponentBase {
 public:
  CommandInput(std::string &value);

  ftxui::Element OnRender() override;

 private:
  ftxui::Box box_;

  static ftxui::Element Transform(ftxui::InputState state);
};

}  // namespace component
}  // namespace tui

#endif
