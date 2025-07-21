#ifndef TUI_COMPONENT_INPUT_HPP
#define TUI_COMPONENT_INPUT_HPP

#include <functional>
#include <string>
#include <string_view>

#include <ftxui/component/component_base.hpp>
#include <ftxui/component/component_options.hpp>

namespace tui {
namespace component {

struct InputOption {
  std::function<void()> on_enter = [] {};
  std::function<void()> on_change = [] {};
};

class Input : public ftxui::ComponentBase {
 public:
  const std::string_view label;

  Input(const std::string_view label, std::string &value, InputOption options);
  Input(const std::string_view label, std::string &value,
        bool multiline = false);

  Input(const std::string_view label, std::string &value, bool multiline,
        InputOption options);

  ftxui::Element OnRender() override;

 private:
  ftxui::Box box_;

  ftxui::Element Transform(ftxui::InputState state);
};

}  // namespace component
}  // namespace tui

#endif
