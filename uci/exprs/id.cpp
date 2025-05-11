#include <memory>
#include <string_view>

#include <uci/expr.hpp>
#include <uci/parser.hpp>
#include <uci/types.hpp>

using namespace uci;

std::unique_ptr<Expr> Parser::ID() {
  std::unique_ptr<expr::ID> expr;
  const std::string_view msg("Expected author or name after 'id'.");

  const auto &token = Consume(WORD, msg);
  const auto &literal = std::get<std::string_view>(token.literal);

  if (literal == "author") {
    expr = std::make_unique<expr::ID>(expr::ID::Type::AUTHOR);
  } else if (literal == "name") {
    expr = std::make_unique<expr::ID>(expr::ID::Type::NAME);
  } else {
    throw Error(token, msg);
  }

  {
    const auto &token = Advance();

    current_ = tokens_.size();
    expr->value = token.lexeme.data();
  }

  return expr;
}
