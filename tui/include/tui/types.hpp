#ifndef TUI_TYPES_HPP
#define TUI_TYPES_HPP

#include <cstdint>

namespace tui {
namespace engine {

using Flags = std::uint8_t;

constexpr Flags UCI = static_cast<Flags>(1) << 0;
constexpr Flags READY = static_cast<Flags>(1) << 1;
constexpr Flags CLEAR_HASH = static_cast<Flags>(1) << 2;

}  // namespace engine
}  // namespace tui

#endif
