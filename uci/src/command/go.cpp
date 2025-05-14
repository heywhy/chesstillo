#include <format>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>

#include <uci/command.hpp>
#include <uci/parser.hpp>
#include <uci/types.hpp>

using namespace uci;

std::unique_ptr<Command> Parser::Go() {
  std::unique_ptr<command::Go> command = std::make_unique<command::Go>();

  std::unordered_map<std::string_view, int *> prop_map = {
      {"wsec", &command->wsec},           {"bsec", &command->bsec},
      {"winc", &command->winc},           {"binc", &command->binc},
      {"movestogo", &command->movestogo}, {"depth", &command->depth},
      {"nodes", &command->nodes},         {"mate", &command->mate},
  };

maybe_return: {
  if (IsAtEnd()) {
    return command;
  }

  goto command;
}

command: {
  std::string_view msg = "Expected ponder, wsec, bsec, winc, binc, movestogo, "
                         "depth, nodes, mate or infinite.";

  const auto &token = Consume(WORD, msg);
  const auto &literal = std::get<std::string_view>(token.literal);

  if (literal == "ponder") {
    command->ponder = true;
    goto maybe_return;
  } else if (literal == "infinite") {
    command->infinite = true;
    goto maybe_return;
  } else if (prop_map.contains("wsec")) {
    int *address = prop_map[literal];
    std::string msg = std::format("Expected number after '{}'", literal);

    const auto &token = Consume(NUMBER, msg);
    const auto &literal = std::get<int>(token.literal);

    *address = literal;
    goto maybe_return;
  }

  throw Error(token, msg);
}

  return command;
}
