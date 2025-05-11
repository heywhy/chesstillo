#include <format>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>

#include <uci/expr.hpp>
#include <uci/parser.hpp>
#include <uci/types.hpp>

using namespace uci;

std::unique_ptr<Expr> Parser::Go() {
  std::unique_ptr<expr::Go> expr = std::make_unique<expr::Go>();

  std::unordered_map<std::string_view, int *> prop_map = {
      {"wsec", &expr->wsec},           {"bsec", &expr->bsec},
      {"winc", &expr->winc},           {"binc", &expr->binc},
      {"movestogo", &expr->movestogo}, {"depth", &expr->depth},
      {"nodes", &expr->nodes},         {"mate", &expr->mate},
  };

maybe_return: {
  if (IsAtEnd()) {
    return expr;
  }

  goto command;
}

command: {
  std::string_view msg = "Expected ponder, wsec, bsec, winc, binc, movestogo, "
                         "depth, nodes, mate or infinite.";

  const auto &token = Consume(WORD, msg);
  const auto &literal = std::get<std::string_view>(token.literal);

  if (literal == "ponder") {
    expr->ponder = true;
    goto maybe_return;
  } else if (literal == "infinite") {
    expr->infinite = true;
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

  return expr;
}
