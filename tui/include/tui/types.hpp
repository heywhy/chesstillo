#ifndef TUI_TYPES_HPP
#define TUI_TYPES_HPP

#include <cstdint>

namespace tui {

using EngineAttrs = std::uint8_t;

namespace attrs {

constexpr EngineAttrs UCI = static_cast<EngineAttrs>(1) << 0;
constexpr EngineAttrs READY = static_cast<EngineAttrs>(1) << 1;
constexpr EngineAttrs CLEAR_HASH = static_cast<EngineAttrs>(1) << 2;

}  // namespace attrs
}  // namespace tui

#endif
