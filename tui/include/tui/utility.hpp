#ifndef TUI_UTILITY_HPP
#define TUI_UTILITY_HPP

#include <memory>
#include <type_traits>

#include <ftxui/component/component_base.hpp>

#include <tui/contracts.hpp>

#define NAVIGATE(component)                               \
  {                                                       \
    auto screen = ftxui::ScreenInteractive::Fullscreen(); \
    screen.Loop(component);                               \
  }

#define RENDER(screen, node) \
  screen.Clear();            \
  ftxui::Render(screen, node);

namespace tui {

void Navigate(const ftxui::Component &component);

template <class T, class... Args>
std::shared_ptr<T> Make(Args &&...args) {
  auto ptr = std::make_shared<T>(std::forward<Args>(args)...);

  if constexpr (std::is_base_of_v<tui::HasKeymaps, T>) {
    HasKeymaps::Bind(ptr.get());
  }

  return std::move(ptr);
}

}  // namespace tui

#endif
