#ifndef TUI_CONFIG_HPP
#define TUI_CONFIG_HPP

#include <map>
#include <string>
#include <vector>

#include <uci/uci.hpp>

using namespace uci;

namespace tui {
class Config {};

struct EngineOption {
  OptionType type;
  command::Option::Value value;
  int min;
  int max;
  std::vector<std::string> vars;
};

typedef std::map<std::string, EngineOption> EngineOptions;

} // namespace tui

#endif
