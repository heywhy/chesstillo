#include <memory>
#include <string>
#include <string_view>
#include <type_traits>
#include <variant>

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

    switch (value_token.type) {
    case NUMBER:
      command->value = std::get<int>(value_token.literal);
      break;
    case BOOLEAN:
      command->value = std::get<bool>(value_token.literal);
      break;
    case WORD:
      command->value = std::get<std::string_view>(value_token.literal);
      break;
    default:
      Error(value_token);
    }
  }

  return command;
}

std::string command::SetOption::ToString() const {
  std::string str("setoption id ");

  str.append(id);

  std::visit(
      [&str](const auto &value) {
        using T = std::decay_t<decltype(value)>;

        if constexpr (std::is_same_v<T, std::string_view>) {
          if (!value.empty()) {
            str.append(" value ").append(value);
          }
        }

        if constexpr (std::is_integral_v<T>) {
          str.append(" value ").append(std::to_string(value));
        }
      },
      value);

  return str;
}
