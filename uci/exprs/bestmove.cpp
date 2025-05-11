#include <memory>
#include <string_view>

#include <uci/expr.hpp>
#include <uci/parser.hpp>
#include <uci/types.hpp>

using namespace uci;

std::unique_ptr<Expr> Parser::BestMove() {
  const auto &token = Consume(WORD, "Expected best move.");
  const auto &literal = std::get<std::string_view>(token.literal);
  std::unique_ptr<expr::BestMove> expr =
      std::make_unique<expr::BestMove>(literal);

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

      expr->ponder = literal;
    }
  }

  return expr;
}
