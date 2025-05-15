#include <memory>
#include <string_view>

#include <uci/command.hpp>
#include <uci/parser.hpp>
#include <uci/types.hpp>

using namespace uci;

std::unique_ptr<Command> Parser::ID() {
  std::unique_ptr<command::ID> command;
  const std::string_view msg("Expected author or name after 'id'.");

  const auto &token = Consume(WORD, msg);
  const auto &literal = std::get<std::string_view>(token.literal);

  if (literal == "author") {
    command = std::make_unique<command::ID>(command::ID::Type::AUTHOR);
  } else if (literal == "name") {
    command = std::make_unique<command::ID>(command::ID::Type::NAME);
  } else {
    throw Error(token, msg);
  }

  {
    const auto &token = Advance();

    current_ = tokens_.size();
    command->value = token.lexeme.data();
  }

  return command;
}

std::string command::ID::ToString() const {
  std::string str("id ");

  switch (type) {
  case AUTHOR:
    str.append("author ");
    break;

  case NAME:
    str.append("name ");
    break;
  }

  str.append(value);

  return str;
}
