#include <algorithm>
#include <array>
#include <memory>
#include <string_view>

#include "uci/command.hpp"
#include "uci/parser.hpp"
#include "uci/types.hpp"

namespace uci {
std::unique_ptr<Command> Parser::Register() {
  static std::array<std::string_view, 3> M = {"later", "name", "code"};
  const std::string_view msg("Expected later, name or code after 'register'.");

  const auto &token = Consume(TokenType::WORD, msg);
  const auto &literal = std::get<std::string_view>(token.literal);

  auto it = std::find(M.begin(), M.end(), literal);

  if (it == M.end()) {
    throw Error(token, msg);
  }

  std::unique_ptr<command::Register> command =
      std::make_unique<command::Register>();

  if (literal == "later") {
    command->later = true;

    return command;
  } else if (literal == "name") {
    goto set_name;
  } else if (literal == "code") {
    goto set_code;
  }

  throw Error(token, "Expected name or code.");

set_attr: {
  const auto &token = Consume(TokenType::WORD, msg);
  const auto &literal = std::get<std::string_view>(token.literal);

  if (literal == "name") {
    goto set_name;
  } else if (literal == "code") {
    goto set_code;
  } else {
    throw Error(token, "Expected name or code.");
  }
}

set_name: {
  while (Match(TokenType::WORD)) {
    const auto &token = Previous();
    const auto &literal = std::get<std::string_view>(token.literal);

    if (literal == "code") {
      goto set_code;
    }

    command->name.append(literal).append(" ");
  }

  goto maybe_return;
}

set_code: {
  const auto &token = Consume(TokenType::WORD, msg);
  const auto &literal = std::get<std::string_view>(token.literal);

  command->code = literal;

  goto maybe_return;
}

maybe_return: {
  if (!command->name.empty()) {
    command->name.resize(command->name.size() - 1);
  }

  if (IsAtEnd()) {
    return command;
  }

  goto set_attr;
}

  return command;
}

std::string command::Register::ToString() const {
  std::string str("register ");

  if (later) {
    str.append("later");

    return str;
  }

  str.append("name ").append(name);
  str.append("code ").append(code);

  return str;
}
}  // namespace uci
