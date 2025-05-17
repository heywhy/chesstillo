#ifndef TUI_SCREEN_ANALYZE_HPP
#define TUI_SCREEN_ANALYZE_HPP

#include <string>

#include <ftxui/component/component_base.hpp>
#include <ftxui/component/event.hpp>
#include <tui/theme.hpp>

namespace tui {
namespace screen {

class Analyze : public ftxui::ComponentBase {
public:
  Analyze(const Theme &theme);

  bool OnEvent(ftxui::Event) override;
  ftxui::Element OnRender() override;

private:
  const Theme &theme_;

  std::string fen_;
  std::string pgn_;

  ftxui::Component MakeContainer();
};
} // namespace screen
} // namespace tui

#endif
