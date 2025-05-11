#include <memory>
#include <set>
#include <string_view>

#include <uci/expr.hpp>
#include <uci/parser.hpp>
#include <uci/types.hpp>

using namespace uci;

#define IS_ATTR(token)                                                         \
  token.type == WORD &&M.contains(std::get<std::string_view>(token.literal))

// TODO: check if the token is a valid move string or abort
#define CONSUME_MOVES(list, error)                                             \
  while (!IsAtEnd()) {                                                         \
    if (IS_ATTR(Peek())) {                                                     \
      goto decide;                                                             \
    }                                                                          \
    const Token &token = Consume(WORD, error);                                 \
    const auto &literal = std::get<std::string_view>(token.literal);           \
    list.emplace_back(literal);                                                \
  }

std::unique_ptr<Expr> Parser::Info() {
  std::unique_ptr<expr::Info> expr = std::make_unique<expr::Info>();

  static std::set<std::string_view> M = {
      "depth",      "seldepth", "time",     "nodes",          "pv",
      "multipv",    "score",    "currmove", "currmovenumber", "hashfull",
      "nps",        "tbhits",   "sbhits",   "cpuload",        "string",
      "refutation", "currline"};

decide: {
  const auto &token = Consume(WORD, "Unexpected token.");
  auto &literal = std::get<std::string_view>(token.literal);

  if (literal == "depth") {
    goto depth;
  } else if (literal == "seldepth") {
    goto seldepth;
  } else if (literal == "time") {
    goto time;
  } else if (literal == "nodes") {
    goto nodes;
  } else if (literal == "pv") {
    goto pv;
  } else if (literal == "multipv") {
    goto multipv;
  } else if (literal == "score") {
    goto score;
  } else if (literal == "currmove") {
    goto currmove;
  } else if (literal == "currmovenumber") {
    goto currmovenumber;
  } else if (literal == "hashfull") {
    goto hashfull;
  } else if (literal == "nps") {
    goto nps;
  } else if (literal == "tbhits") {
    goto tbhits;
  } else if (literal == "sbhits") {
    goto sbhits;
  } else if (literal == "cpuload") {
    goto cpuload;
  } else if (literal == "string") {
    goto string;
  } else if (literal == "refutation") {
    goto refutation;
  } else if (literal == "currline") {
    goto currline;
  }

  throw Error(token, "Unexpected token.");
}

maybe_return: {
  if (IsAtEnd()) {
    return expr;
  }

  goto decide;
}

depth: {
  const Token &token = Consume(NUMBER, "Expect number after 'depth'.");

  expr->depth = std::get<int>(token.literal);

  goto maybe_return;
}

seldepth: {
  const Token &token = Consume(NUMBER, "Expect number after 'seldepth'.");

  expr->seldepth = std::get<int>(token.literal);

  goto maybe_return;
}

time: {
  const Token &token = Consume(NUMBER, "Expect number after 'time'.");

  expr->time = std::get<int>(token.literal);

  goto maybe_return;
}

nodes: {
  const Token &token = Consume(NUMBER, "Expect number after 'nodes'.");

  expr->nodes = std::get<int>(token.literal);

  goto maybe_return;
}

pv: {
  CONSUME_MOVES(expr->pv, "Expect move after 'pv'.");

  goto maybe_return;
}

multipv: {
  const Token &token = Consume(NUMBER, "Expect number after 'multipv'.");

  expr->multipv = std::get<int>(token.literal);

  goto maybe_return;
}

score: {
  const Token &token = Consume(WORD, "Expect word after 'score'.");
  std::string_view type = std::get<std::string_view>(token.literal);

  if (type == "cp") {
    const Token &token = Consume(NUMBER, "Expect number after 'cp'.");

    expr->score.type = expr::Info::Score::CP;
    expr->score.value = std::get<int>(token.literal);
  } else if (type == "mate") {
    const Token &token = Consume(NUMBER, "Expect number after 'mate'.");

    expr->score.type = expr::Info::Score::MATE;
    expr->score.value = std::get<int>(token.literal);
  } else if (type == "lowerbound") {
    expr->score.type = expr::Info::Score::LOWER_BOUND;
  } else if (type == "upperbound") {
    expr->score.type = expr::Info::Score::UPPER_BOUND;
  } else {
    throw Error(token,
                "Expect cp, mate, lowerbound or upperbound after 'score'.");
  }

  goto maybe_return;
}

  // TODO: check if it's valid move
currmove: {
  const Token &token = Consume(WORD, "Expect move after 'currmove'.");

  expr->currmove = std::get<std::string_view>(token.literal);

  goto maybe_return;
}

currmovenumber: {
  const Token &token = Consume(NUMBER, "Expect number after 'currmovenumber'.");

  expr->currmovenumber = std::get<int>(token.literal);

  if (expr->currmovenumber < 1) {
    throw Error(token, "Expect currmovenumber to be greater than 0");
  }

  goto maybe_return;
}

hashfull: {
  const Token &token = Consume(NUMBER, "Expect number after 'hashfull'.");

  expr->hashfull = std::get<int>(token.literal);

  goto maybe_return;
}

nps: {
  const Token &token = Consume(NUMBER, "Expect number after 'nps'.");

  expr->nps = std::get<int>(token.literal);

  goto maybe_return;
}

tbhits: {
  const Token &token = Consume(NUMBER, "Expect number after 'tbhits'.");

  expr->tbhits = std::get<int>(token.literal);

  goto maybe_return;
}

sbhits: {
  const Token &token = Consume(NUMBER, "Expect number after 'sbhits'.");

  expr->sbhits = std::get<int>(token.literal);

  goto maybe_return;
}

cpuload: {
  const Token &token = Consume(NUMBER, "Expect number after 'cpuload'.");

  expr->cpuload = std::get<int>(token.literal);

  goto maybe_return;
}

string: {
  const Token &token = Advance();

  // INFO: as per spec, rest of the line is considered the string value
  current_ = tokens_.size();
  expr->string = token.lexeme.data();
}

refutation: {
  CONSUME_MOVES(expr->refutation, "Expect move after 'refutation'.");

  goto maybe_return;
}

currline: {
  const Token &token = Consume(NUMBER, "Expect number after 'currline'.");

  expr->currline.cpunr = std::get<int>(token.literal);

  if (expr->currline.cpunr < 1) {
    throw Error(token, "Expect currline.cpunr to be greater than 0");
  }

  CONSUME_MOVES(expr->currline.moves, "Expect move after cpu number.");

  goto maybe_return;
}

  return expr;
}
