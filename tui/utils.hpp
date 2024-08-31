#ifndef TUI_UTILS_HPP
#define TUI_UTILS_HPP

#include <chesstillo/types.hpp>
#include <ftxui/component/component_base.hpp>

void HookQuitEvent(ftxui::Component &component);
bool ToString(char *text, Move const &move, Color turn);
#endif
