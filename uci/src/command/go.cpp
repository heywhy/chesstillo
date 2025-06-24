#include <cstdint>
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
      {"movetime", &command->movetime}};

maybe_return: {
  if (IsAtEnd()) {
    return command;
  }

  goto command;
}

command: {
  std::string_view msg =
      "Expected searchmoves, ponder, wsec, bsec, winc, binc, movestogo, "
      "depth, nodes, mate, movetime or infinite.";

  const auto &token = Consume(TokenType::WORD, msg);
  const auto &literal = std::get<std::string_view>(token.literal);

  if (literal == "ponder") {
    command->ponder = true;
    goto maybe_return;
  } else if (literal == "infinite") {
    command->infinite = true;
    goto maybe_return;
  } else if (literal == "searchmoves") {
    goto searchmoves;
  } else if (prop_map.contains(literal)) {
    int *address = prop_map[literal];
    std::string msg = std::format("Expected number after '{}'", literal);

    const auto &token = Consume(TokenType::NUMBER, msg);
    const auto &literal = std::get<std::int64_t>(token.literal);

    *address = literal;
    goto maybe_return;
  }

  throw Error(token, msg);
}

searchmoves: {
  while (Check(TokenType::WORD) &&
         !(prop_map.contains(std::get<std::string_view>(Peek().literal)))) {
    const auto &token = Advance();
    const auto &literal = std::get<std::string_view>(token.literal);

    command->searchmoves.push_back(literal);
  }

  goto maybe_return;
}

  return command;
}

std::string command::Go::ToString() const {
  std::string str("go ");

  if (ponder) {
    str.append("ponder");
  }

  if (wsec >= 0) {
    str.append(" wsec ").append(std::to_string(wsec));
  }

  if (bsec >= 0) {
    str.append(" bsec ").append(std::to_string(bsec));
  }

  if (winc > 0) {
    str.append(" winc ").append(std::to_string(winc));
  }

  if (binc > 0) {
    str.append(" binc ").append(std::to_string(binc));
  }

  if (movestogo > 0) {
    str.append(" movestogo ").append(std::to_string(movestogo));
  }

  if (depth > 0) {
    str.append(" depth ").append(std::to_string(depth));
  }

  if (nodes > 0) {
    str.append(" nodes ").append(std::to_string(nodes));
  }

  if (mate > 0) {
    str.append(" mate ").append(std::to_string(mate));
  }

  if (movetime > 0) {
    str.append(" movetime ").append(std::to_string(movetime));
  }

  if (infinite) {
    str.append(" infinite ");
  }

  if (searchmoves.empty()) {
    return str;
  }

  str.append(" searchmoves ");

  for (const auto &move : searchmoves) {
    str.append(" ").append(move);
  }

  return str;
}
