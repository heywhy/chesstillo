#ifndef GUI_UTILS_HPP

#define GUI_UTILS_HPP

#include <cstdint>
#include <ftxui/component/component_base.hpp>

class OnSelectSquare {
public:
  virtual void OnSelect(std::uint8_t index) = 0;
};

void HookQuitEvent(ftxui::Component &component);

#endif
