#ifndef TUI_TYPES_HPP
#define TUI_TYPES_HPP

#include <cstdint>
#include <utility>

#include "engine/types.hpp"

namespace tui {

using EngineAttrs = std::uint8_t;
using PieceFontMap = std::pair<const char *, engine::Color>;

namespace attrs {

constexpr EngineAttrs UCI = static_cast<EngineAttrs>(1) << 0;
constexpr EngineAttrs READY = static_cast<EngineAttrs>(1) << 1;
constexpr EngineAttrs CLEAR_HASH = static_cast<EngineAttrs>(1) << 2;

}  // namespace attrs
}  // namespace tui

#endif
