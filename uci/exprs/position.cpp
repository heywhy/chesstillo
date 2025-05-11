#include <memory>
#include <string>
#include <string_view>

#include <uci/expr.hpp>
#include <uci/parser.hpp>
#include <uci/types.hpp>

using namespace uci;

std::unique_ptr<Expr> Parser::Position() {
  std::unique_ptr<expr::Position> expr;
  const auto &token = Consume(WORD, "Expected 'startpos' or 'fen'.");
  const auto &literal = std::get<std::string_view>(token.literal);

  if (literal == "startpos") {
    goto startpos;
  } else if (literal == "fen") {
    goto fen;
  }

startpos: {
  expr = std::make_unique<expr::Position>(literal);

  if (IsAtEnd()) {
    return expr;
  }

  const std::string_view msg = "Expected 'moves'.";

  const auto &token = Consume(WORD, msg);
  const auto &literal = std::get<std::string_view>(token.literal);

  if (literal != "moves") {
    throw Error(token, msg);
  }

  goto moves;
}

fen: {
  std::string fen;
  std::string_view msg("Expected 'fenstring'.");

  const auto &token = Consume(WORD, msg);
  const auto &literal = std::get<std::string_view>(token.literal);

  fen.append(literal).append(" ");

  while (!IsAtEnd()) {
    if (Match(NUMBER)) {
      const auto &literal = std::get<int>(Previous().literal);

      fen.append(std::to_string(literal)).append(" ");
      continue;
    }

    const auto &token = Consume(WORD, msg);
    const auto &literal = std::get<std::string_view>(token.literal);

    if (literal == "moves") {
      fen.resize(fen.size() - 1);
      expr = std::make_unique<expr::Position>(fen);
      goto moves;
    } else {
      fen.append(literal).append(" ");
    }
  }

  fen.resize(fen.size() - 1);
  expr = std::make_unique<expr::Position>(fen);
}

moves: {
  while (!IsAtEnd()) {
    const auto &token = Consume(WORD, "Expected 'moves'.");
    const auto &literal = std::get<std::string_view>(token.literal);

    expr->moves.emplace_back(literal);
  }
}

  return expr;
}
