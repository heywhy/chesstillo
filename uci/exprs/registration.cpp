#include <memory>
#include <string_view>

#include <uci/expr.hpp>
#include <uci/parser.hpp>
#include <uci/types.hpp>

using namespace uci;

std::unique_ptr<Expr> Parser::Registration() {
  const auto &token = Consume(WORD, "Expected checking, ok or error.");
  const auto &literal = std::get<std::string_view>(token.literal);

  if (!kStatus.contains(literal)) {
    throw Error(token, "Unexpected token.");
  }

  return std::make_unique<expr::Registration>(kStatus[literal]);
}
