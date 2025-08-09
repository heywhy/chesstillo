#ifndef TUI_COMPONENT_SQUARE_HPP
#define TUI_COMPONENT_SQUARE_HPP

#include <functional>

#include <ftxui/component/component_base.hpp>

#include "tui/theme.hpp"

namespace tui {
namespace component {
class Square : public ftxui::ComponentBase {
 public:
  using OnClick = std::function<void(Square *)>;

  const int index;

  Square(const tui::Theme &, int index, OnClick on_click = [](auto) {});

  ftxui::Element OnRender() override;
  bool OnEvent(ftxui::Event event) override;

  bool Focusable() const override { return true; }

  inline bool Empty() const { return piece_ == '\0'; }
  inline void SetPiece(const char piece) { piece_ = piece; }

 private:
  char piece_;
  bool hover_;
  bool selected_;
  ftxui::Box box_;
  const Theme &theme_;
  const OnClick on_click_;

  inline void Toggle() { selected_ = !selected_; }

  friend class Chessboard;
};
}  // namespace component

}  // namespace tui

#endif
