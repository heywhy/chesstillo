#include <memory>
#include <string_view>

#include <uci/command.hpp>
#include <uci/parser.hpp>
#include <uci/types.hpp>

using namespace uci;

std::unique_ptr<Command> Parser::BestMove() {
  const auto &token = Consume(WORD, "Expected best move.");
  const auto &literal = std::get<std::string_view>(token.literal);
  std::unique_ptr<command::BestMove> command =
      std::make_unique<command::BestMove>(literal);

  if (!IsAtEnd()) {
    const std::string_view msg("Expected ponder.");

    const auto &token = Consume(WORD, msg);
    const auto &literal = std::get<std::string_view>(token.literal);

    if (literal != "ponder") {
      throw Error(token, msg);
    }

    {
      const auto &token = Consume(WORD, "Expected move to ponder");
      const auto &literal = std::get<std::string_view>(token.literal);

      command->ponder = literal;
    }
  }

  return command;
}

std::string command::BestMove::ToString() const {
  std::string str("bestmove ");

  str.append(move);

  if (!ponder.empty()) {
    str.append(" ponder ").append(ponder);
  }

  return str;
}
