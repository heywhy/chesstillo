#include <memory>
#include <string>
#include <string_view>

#include "uci/command.hpp"
#include "uci/parser.hpp"
#include "uci/types.hpp"

namespace uci {
std::unique_ptr<Command> Parser::Debug() {
  std::unique_ptr<command::Debug> command;
  const auto &token = Consume(TokenType::WORD, "Expected on or off.");
  const auto &literal = std::get<std::string_view>(token.literal);

  if (literal == "on") {
    command.reset(new command::Debug(true));
  } else if (literal == "off") {
    command.reset(new command::Debug(false));
  } else {
    throw Error(token);
  }

  return command;
}

std::string command::Debug::ToString() const {
  std::string str("debug ");

  return value ? str.append("on") : str.append("off");
}
}  // namespace uci
