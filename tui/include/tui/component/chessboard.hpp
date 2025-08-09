#ifndef TUI_COMPONENT_CHESSBOARD_HPP
#define TUI_COMPONENT_CHESSBOARD_HPP

#include <functional>

#include <ftxui/component/component_base.hpp>

#include "tui/component/square.hpp"
#include "tui/theme.hpp"

namespace tui {
namespace component {
class Chessboard : public ftxui::ComponentBase {
 public:
  using OnSelect = std::function<void(Square *)>;

  Chessboard(
      const Theme &theme,
      OnSelect on_select = [](Square *square) { square->Toggle(); });

  ftxui::Element OnRender() override;
  bool OnEvent(ftxui::Event event) override;
  ftxui::Component ActiveChild() override;
  void SetActiveChild(ftxui::ComponentBase *child) override;

  void ToggleSquare(Square *square);

  inline void SetPiece(const char piece, const int index) {
    const auto square = static_cast<component::Square *>(ChildAt(index).get());

    square->SetPiece(piece);
  }

 private:
  ftxui::Box box_;
  int selector_;
  OnSelect on_select_;

  void MoveSelector(int dir);
  bool OnKeyEvent(ftxui::Event &);
  bool OnMouseEvent(ftxui::Event &);
};
}  // namespace component
}  // namespace tui

#endif
