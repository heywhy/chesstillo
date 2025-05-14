#include <memory>
#include <string_view>

#include <uci/command.hpp>
#include <uci/parser.hpp>
#include <uci/types.hpp>

using namespace uci;

std::unique_ptr<Command> Parser::Debug() {
  std::unique_ptr<command::Debug> command;
  const auto &token = Consume(WORD, "Expected on or off.");
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
