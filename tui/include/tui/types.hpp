#ifndef TUI_TYPES_HPP
#define TUI_TYPES_HPP

#include <cstdint>

namespace tui {
namespace engine {

using Flags = std::uint8_t;

constexpr Flags UCI = 1 << 0;
constexpr Flags READY = 1 << 1;
constexpr Flags PONDER = 1 << 2;

}  // namespace engine
}  // namespace tui

#endif
