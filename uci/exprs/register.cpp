#include <algorithm>
#include <array>
#include <memory>
#include <string_view>

#include <uci/expr.hpp>
#include <uci/parser.hpp>
#include <uci/types.hpp>

using namespace uci;

std::unique_ptr<Expr> Parser::Register() {
  static std::array<std::string_view, 3> M = {"later", "name", "code"};
  const std::string_view msg("Expected later, name or code after 'register'.");

  const auto &token = Consume(WORD, msg);
  const auto &literal = std::get<std::string_view>(token.literal);

  auto it = std::find(M.begin(), M.end(), literal);

  if (it == M.end()) {
    throw Error(token, msg);
  }

  std::unique_ptr<expr::Register> expr = std::make_unique<expr::Register>();

  if (literal == "later") {
    expr->later = true;

    return expr;
  } else if (literal == "name") {
    goto set_name;
  } else if (literal == "code") {
    goto set_code;
  }

  throw Error(token, "Expected name or code.");

set_attr: {
  const auto &token = Consume(WORD, msg);
  const auto &literal = std::get<std::string_view>(token.literal);

  if (literal == "name") {
    goto set_name;
  } else if (literal == "code") {
    goto set_code;
  } else {
    throw Error(token, "Expected name or code.");
  }
}

set_name: {
  while (Match(WORD)) {
    const auto &token = Previous();
    const auto &literal = std::get<std::string_view>(token.literal);

    if (literal == "code") {
      goto set_code;
    }

    expr->name.append(literal).append(" ");
  }

  goto maybe_return;
}

set_code: {
  const auto &token = Consume(WORD, msg);
  const auto &literal = std::get<std::string_view>(token.literal);

  expr->code = literal;

  goto maybe_return;
}

maybe_return: {
  if (!expr->name.empty()) {
    expr->name.resize(expr->name.size() - 1);
  }

  if (IsAtEnd()) {
    return expr;
  }

  goto set_attr;
}

  return expr;
}
