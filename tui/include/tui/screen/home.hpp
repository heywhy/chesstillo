#ifndef TUI_SCREEN_HOME_HPP
#define TUI_SCREEN_HOME_HPP

#include <ftxui/component/component_base.hpp>
#include <ftxui/component/event.hpp>
#include <tui/theme.hpp>

namespace tui {
namespace screen {

class Home : public ftxui::ComponentBase {
public:
  Home(const Theme &theme);
  bool OnEvent(ftxui::Event) override;
  ftxui::Element OnRender() override;

private:
  const Theme &theme_;
};
} // namespace screen
} // namespace tui

#endif
