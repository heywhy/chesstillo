#include <memory>
#include <string>
#include <string_view>

#include <uci/command.hpp>
#include <uci/parser.hpp>
#include <uci/types.hpp>

using namespace uci;

std::unique_ptr<Command> Parser::SetOption() {
  const std::string_view msg("Expected 'name' after 'setoption'.");

  const auto &token = Consume(WORD, msg);
  const auto &literal = std::get<std::string_view>(token.literal);

  if (literal != "name") {
    throw Error(token, msg);
  }

  const auto &id_token = Consume(WORD, "Expected 'id'.");
  const auto &id_literal = std::get<std::string_view>(id_token.literal);

  std::unique_ptr<command::SetOption> command =
      std::make_unique<command::SetOption>(id_literal);

  if (!IsAtEnd()) {
    const auto &token = Consume(WORD, msg);
    const auto &literal = std::get<std::string_view>(token.literal);

    if (literal != "value") {
      throw Error(Previous(), "Expected 'value' after.");
    }

    if (IsAtEnd()) {
      throw Error(Previous(), "Expected a value after 'value'.");
    }

    const auto &value_token = Advance();

    // INFO: set to end of tokens because the remaining tokens should be
    // regarded as the option value
    current_ = tokens_.size();

    command->value = value_token.lexeme.data();
  }

  return command;
}
