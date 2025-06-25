#ifndef TUI_TYPES_HPP
#define TUI_TYPES_HPP

#include <cstdint>

namespace tui {
namespace engine {

using Flag = std::uint8_t;

constexpr Flag UCI = static_cast<Flag>(1) << 0;
constexpr Flag READY = static_cast<Flag>(1) << 1;
constexpr Flag CLEAR_HASH = static_cast<Flag>(1) << 2;

}  // namespace engine
}  // namespace tui

#endif
