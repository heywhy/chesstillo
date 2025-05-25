#ifndef TUI_COMPONENT_SWITCH_HPP
#define TUI_COMPONENT_SWITCH_HPP

#include <array>
#include <functional>
#include <string_view>

#include <ftxui/component/component.hpp>

namespace tui {
namespace component {

class Switch : public ftxui::ComponentBase {
 public:
  using Labels = std::array<std::string_view, 2>;
  using OnChange = std::function<void()>;

  const Labels labels;

  Switch(bool &on);
  Switch(bool &on, OnChange on_change);
  Switch(const Labels &labels, bool &on, const OnChange on_change = [] {});

 private:
  int selected_;
  const OnChange on_change_;
};

}  // namespace component
}  // namespace tui

#endif
