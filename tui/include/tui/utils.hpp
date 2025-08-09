#ifndef TUI_UTILITY_HPP
#define TUI_UTILITY_HPP

#include <algorithm>
#include <cctype>
#include <memory>
#include <string>
#include <type_traits>

#include <ftxui/component/component_base.hpp>

#include "tui/contracts.hpp"

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

// trim from start (in place)
inline void ltrim(std::string &s) {
  s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
            return !std::isspace(ch);
          }));
}

// trim from end (in place)
inline void rtrim(std::string &s) {
  s.erase(std::find_if(s.rbegin(), s.rend(),
                       [](unsigned char ch) { return !std::isspace(ch); })
              .base(),
          s.end());
}

inline void trim(std::string &s) {
  rtrim(s);
  ltrim(s);
}

}  // namespace tui

#endif
