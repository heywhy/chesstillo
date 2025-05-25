#ifndef TUI_COMPONENT_CHESSBOARD_HPP
#define TUI_COMPONENT_CHESSBOARD_HPP

#include <ftxui/component/component_base.hpp>

#include <tui/component/square.hpp>
#include <tui/theme.hpp>

namespace tui {
namespace component {
class Chessboard : public ftxui::ComponentBase {
 public:
  Chessboard(const Theme &theme);

  ftxui::Element OnRender() override;
  bool OnEvent(ftxui::Event event) override;
  ftxui::Component ActiveChild() override;
  void SetActiveChild(ftxui::ComponentBase *child) override;

 private:
  ftxui::Box box_;
  int selector_;

  void MoveSelector(int dir);
  bool OnKeyEvent(ftxui::Event &);
  bool OnMouseEvent(ftxui::Event &);
};
}  // namespace component
}  // namespace tui

#endif
