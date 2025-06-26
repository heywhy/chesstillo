#include <cstdint>
#include <memory>
#include <string>
#include <string_view>

#include <uci/command.hpp>
#include <uci/parser.hpp>
#include <uci/types.hpp>

namespace uci {
std::unique_ptr<Command> Parser::Position() {
  std::unique_ptr<command::Position> command;
  const auto &token = Consume(TokenType::WORD, "Expected 'startpos' or 'fen'.");
  const auto &literal = std::get<std::string_view>(token.literal);

  if (literal == "startpos") {
    goto startpos;
  } else if (literal == "fen") {
    goto fen;
  }

startpos: {
  command = std::make_unique<command::Position>(literal);

  if (IsAtEnd()) {
    return command;
  }

  const std::string_view msg = "Expected 'moves'.";

  const auto &token = Consume(TokenType::WORD, msg);
  const auto &literal = std::get<std::string_view>(token.literal);

  if (literal != "moves") {
    throw Error(token, msg);
  }

  goto moves;
}

fen: {
  std::string fen;
  std::string_view msg("Expected 'fenstring'.");

  const auto &token = Consume(TokenType::WORD, msg);
  const auto &literal = std::get<std::string_view>(token.literal);

  fen.append(literal).append(" ");

  while (!IsAtEnd()) {
    if (Match(TokenType::NUMBER)) {
      const auto &literal = std::get<std::int64_t>(Previous().literal);

      fen.append(std::to_string(literal)).append(" ");
      continue;
    }

    const auto &token = Consume(TokenType::WORD, msg);
    const auto &literal = std::get<std::string_view>(token.literal);

    if (literal == "moves") {
      fen.resize(fen.size() - 1);
      command = std::make_unique<command::Position>(fen);
      goto moves;
    } else {
      fen.append(literal).append(" ");
    }
  }

  fen.resize(fen.size() - 1);
  command = std::make_unique<command::Position>(fen);
}

moves: {
  while (!IsAtEnd()) {
    const auto &token = Consume(TokenType::WORD, "Expected 'moves'.");
    const auto &literal = std::get<std::string_view>(token.literal);

    command->moves.emplace_back(literal);
  }
}

  return command;
}

std::string command::Position::ToString() const {
  std::string str("position ");

  if (input == "startpos") {
    str.append(input);
  } else {
    str.append("fen ").append(input);
  }

  if (moves.empty()) {
    return str;
  }

  str.append(" moves");

  for (const auto &move : moves) {
    str.append(" ").append(move);
  }

  return str;
}
}  // namespace uci
