#include <memory>
#include <set>
#include <string_view>
#include <unordered_map>

#include <uci/command.hpp>
#include <uci/parser.hpp>
#include <uci/types.hpp>

using namespace uci;

std::unique_ptr<Command> Parser::Option() {
  std::unique_ptr<command::Option> command =
      std::make_unique<command::Option>();

  static std::set<std::string_view> M = {"name", "type", "default",
                                         "var",  "min",  "max"};

  static std::unordered_map<std::string_view, OptionType> N = {
      {"check", CHECK},   {"spin", SPIN},     {"combo", COMBO},
      {"button", BUTTON}, {"string", STRING},
  };

decide: {
  const auto &token = Consume(WORD);
  const auto &literal = std::get<std::string_view>(token.literal);

  if (literal == "name") {
    goto name;
  } else if (literal == "type") {
    goto type;
  } else if (literal == "default") {
    goto def4ult;
  } else if (literal == "var") {
    goto var;
  } else if (literal == "min") {
    goto min;
  } else if (literal == "max") {
    goto max;
  }

  throw Error(token);
}

maybe_return: {
  if (IsAtEnd()) {
    return command;
  }

  goto decide;
}

name: {
  std::string id;

  const auto &token = Consume(WORD, "Expected name of the option.");
  const auto &literal = std::get<std::string_view>(token.literal);

  id.append(literal).append(" ");

  while (Check(WORD) &&
         !M.contains(std::get<std::string_view>(Peek().literal))) {
    const auto &token = Advance();
    const auto &literal = std::get<std::string_view>(token.literal);

    id.append(literal).append(" ");
  }

  id.resize(id.size() - 1);

  command->id = id;

  goto maybe_return;
}

type: {
  const std::string_view msg("Expected check, spin, combo, button and string.");

  const auto &token = Consume(WORD, msg);
  const auto &literal = std::get<std::string_view>(token.literal);

  if (!N.contains(literal)) {
    throw Error(token, msg);
  }

  command->type = N[literal];
  goto maybe_return;
}

def4ult: {
  if (command->type == SPIN) {
    const auto &token = Consume(NUMBER);
    const auto &literal = std::get<int>(token.literal);

    command->def4ult = literal;
  } else if (command->type == CHECK) {
    const auto &token = Consume(WORD);
    const auto &literal = std::get<std::string_view>(token.literal);

    if (literal == "false") {
      command->def4ult = false;
    } else if (literal == "true") {
      command->def4ult = true;
    } else {
      throw Error(token);
    }
  } else if (command->type == COMBO || command->type == STRING) {
    const auto &token = Consume(WORD);
    const auto &literal = std::get<std::string_view>(token.literal);

    command->def4ult = literal;
  } else {
    throw Error(Previous());
  }

  goto maybe_return;
}

var: {
  const auto &token = Consume(WORD);
  const auto &literal = std::get<std::string_view>(token.literal);

  command->vars.emplace_back(literal);

  goto maybe_return;
}

min: {
  const auto &token = Consume(NUMBER);
  const auto &literal = std::get<int>(token.literal);

  command->min = literal;

  goto maybe_return;
}

max: {
  const auto &token = Consume(NUMBER);
  const auto &literal = std::get<int>(token.literal);

  command->max = literal;

  goto maybe_return;
}

  return command;
}
