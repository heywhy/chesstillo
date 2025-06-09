#ifndef TUI_CONFIG_HPP
#define TUI_CONFIG_HPP

#include <cstdint>
#include <functional>
#include <map>
#include <string>
#include <variant>
#include <vector>

#include <uci/uci.hpp>

namespace tui {
class Config {};

struct EngineOption {
  std::string id;
  uci::OptionType type;
  std::variant<std::string, std::int64_t, bool> value;
  std::int64_t min;
  std::int64_t max;
  std::vector<std::string> vars;

  EngineOption();
  EngineOption(std::function<void(uci::command::SetOption &)> send_command);

  void OnChange();

 private:
  std::function<void(uci::command::SetOption &)> send_command_;
};

typedef std::map<std::string, EngineOption> EngineOptions;

}  // namespace tui

#endif
