#include <memory>
#include <set>
#include <string_view>
#include <unordered_map>

#include <uci/expr.hpp>
#include <uci/parser.hpp>
#include <uci/types.hpp>

using namespace uci;

std::unique_ptr<Expr> Parser::Option() {
  std::unique_ptr<expr::Option> expr = std::make_unique<expr::Option>();

  static std::set<std::string_view> M = {"name", "type", "default",
                                         "var",  "min",  "max"};

  static std::unordered_map<std::string_view, OptionType> N = {
      {"check", CHECK},   {"spin", SPIN},     {"combo", COMBO},
      {"button", BUTTON}, {"string", STRING},
  };

decide: {
  const auto &token = Consume(WORD, "Expected 'name'.");
  const auto &literal = std::get<std::string_view>(token.literal);

  if (literal == "name") {
    goto name;
  } else if (literal == "type") {
    goto type;
  } else if (literal == "default") {
    goto def4ult;
  } else if (literal == "var") {
    goto var;
  } else if (literal == "min") {
    goto min;
  } else if (literal == "max") {
    goto max;
  }

  throw Error(token, "Unexpected token.");
}

maybe_return: {
  if (IsAtEnd()) {
    return expr;
  }

  goto decide;
}

name: {
  std::string id;

  const auto &token = Consume(WORD, "Expected name of the option.");
  const auto &literal = std::get<std::string_view>(token.literal);

  id.append(literal).append(" ");

  while (Check(WORD) &&
         !M.contains(std::get<std::string_view>(Peek().literal))) {
    const auto &token = Advance();
    const auto &literal = std::get<std::string_view>(token.literal);

    id.append(literal).append(" ");
  }

  id.resize(id.size() - 1);

  expr->id = id;

  goto maybe_return;
}

type: {
  const std::string_view msg("Expected check, spin, combo, button and string.");

  const auto &token = Consume(WORD, msg);
  const auto &literal = std::get<std::string_view>(token.literal);

  if (!N.contains(literal)) {
    throw Error(token, msg);
  }

  expr->type = N[literal];
  goto maybe_return;
}

def4ult: {
  if (expr->type == SPIN) {
    const auto &token = Consume(NUMBER, "Unexpected token.");
    const auto &literal = std::get<int>(token.literal);

    expr->def4ult = literal;
  } else if (expr->type == CHECK) {
    const auto &token = Consume(WORD, "Unexpected token.");
    const auto &literal = std::get<std::string_view>(token.literal);

    if (literal == "false") {
      expr->def4ult = false;
    } else if (literal == "true") {
      expr->def4ult = true;
    } else {
      throw Error(token, "Unexpected token.");
    }
  } else if (expr->type == COMBO || expr->type == STRING) {
    const auto &token = Consume(WORD, "Unexpected token.");
    const auto &literal = std::get<std::string_view>(token.literal);

    expr->def4ult = literal;
  } else {
    throw Error(Previous(), "Unexpected token.");
  }

  goto maybe_return;
}

var: {
  const auto &token = Consume(WORD, "Unexpected token.");
  const auto &literal = std::get<std::string_view>(token.literal);

  expr->vars.emplace_back(literal);

  goto maybe_return;
}

min: {
  const auto &token = Consume(NUMBER, "Unexpected token.");
  const auto &literal = std::get<int>(token.literal);

  expr->min = literal;

  goto maybe_return;
}

max: {
  const auto &token = Consume(NUMBER, "Unexpected token.");
  const auto &literal = std::get<int>(token.literal);

  expr->max = literal;

  goto maybe_return;
}

  return expr;
}
