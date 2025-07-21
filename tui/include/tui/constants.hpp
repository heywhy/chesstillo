#ifndef TUI_CONSTANTS_HPP
#define TUI_CONSTANTS_HPP

#include <unordered_map>

#include "types.hpp"

namespace tui {

inline const char *kSpaceChar = "\x20";

extern std::unordered_map<char, PieceFontMap> kPieceFontsMap;

}  // namespace tui
#endif
