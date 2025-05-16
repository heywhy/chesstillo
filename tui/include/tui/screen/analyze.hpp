#ifndef TUI_SCREEN_ANALYZE_HPP
#define TUI_SCREEN_ANALYZE_HPP

#include <string>

#include <ftxui/component/component_base.hpp>
#include <ftxui/component/event.hpp>

namespace tui {
namespace screen {

class Analyze : public ftxui::ComponentBase {
public:
  Analyze();

  bool OnEvent(ftxui::Event) override;
  ftxui::Element OnRender() override;

private:
  std::string fen_;
  std::string pgn_;

  ftxui::Component container_;

  ftxui::Component &InitContainer();
};
} // namespace screen
} // namespace tui

#endif
