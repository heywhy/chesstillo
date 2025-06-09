// dubbed from tuwe. https://github.com/moogion/tuwe
#ifndef TUI_COMPONENT_SCROLL_VIEW_HPP
#define TUI_COMPONENT_SCROLL_VIEW_HPP

#include <ftxui/component/component_base.hpp>

namespace tui {
namespace component {
class ScrollView : public ftxui::ComponentBase {
 public:
  enum Direction { ALL, WIDTH, HEIGHT };

  ScrollView(ftxui::Component child);
  ScrollView(ftxui::Component child, const Direction direction);

  ftxui::Element OnRender() override;
  bool OnEvent(ftxui::Event event) override;
  bool Focusable() const override { return true; }

 private:
  ftxui::Box box_;
  Direction direction_;

  int scroll_width_;
  int scroll_height_;

  int size_width_;
  int size_height_;
};
}  // namespace component

}  // namespace tui

#endif
