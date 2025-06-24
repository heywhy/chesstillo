#include <cstdint>
#include <string>

#include <uci/uci.hpp>

#include <tui/config.hpp>

namespace tui {

EngineOption::EngineOption() : min(0), max(0) {}

uci::command::SetOption EngineOption::ToCommand() const {
  auto command = uci::command::SetOption(id);

  switch (type) {
    case uci::OptionType::CHECK:
      command.value = std::get<bool>(value);
      break;

    case uci::OptionType::SPIN:
      command.value = std::get<std::int64_t>(value);
      break;

    case uci::OptionType::COMBO:
      command.value = std::get<std::string>(value);
      break;

    case uci::OptionType::BUTTON:
      break;

    case uci::OptionType::STRING:
      command.value = std::get<std::string>(value);
      break;
  }

  return command;
}

}  // namespace tui
