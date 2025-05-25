#ifndef TUI_COMPONENT_SQUARE_HPP
#define TUI_COMPONENT_SQUARE_HPP

#include <ftxui/component/component_base.hpp>

#include <tui/theme.hpp>

namespace tui {
namespace component {
class Square : public ftxui::ComponentBase {
 public:
  const int index;

  Square(const tui::Theme &, int index);

  ftxui::Element OnRender() override;
  bool OnEvent(ftxui::Event event) override;

  bool Focusable() const override { return true; }

 private:
  ftxui::Box box_;
  const Theme &theme_;
};
}  // namespace component

}  // namespace tui

#endif
