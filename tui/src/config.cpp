#include <cstdint>
#include <string>

#include <uci/uci.hpp>

#include <tui/config.hpp>

namespace tui {

EngineOption::EngineOption() : min(0), max(0) {}

uci::command::SetOption EngineOption::ToCommand() const {
  auto command = uci::command::SetOption(id);

  switch (type) {
    case uci::CHECK:
      command.value = std::get<bool>(value);
      break;

    case uci::SPIN:
      command.value = std::get<std::int64_t>(value);
      break;

    case uci::COMBO:
      command.value = std::get<std::string>(value);
      break;

    case uci::BUTTON:
      break;

    case uci::STRING:
      command.value = std::get<std::string>(value);
      break;
  }

  return command;
}

}  // namespace tui
