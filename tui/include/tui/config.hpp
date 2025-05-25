#ifndef TUI_CONFIG_HPP
#define TUI_CONFIG_HPP

#include <cstdint>
#include <map>
#include <string>
#include <variant>
#include <vector>

#include <uci/uci.hpp>

namespace tui {
class Config {};

struct EngineOption {
  uci::OptionType type;
  std::variant<std::string, std::int64_t, bool> value;
  int min;
  int max;
  std::vector<std::string> vars;
};

typedef std::map<std::string, EngineOption> EngineOptions;

}  // namespace tui

#endif
