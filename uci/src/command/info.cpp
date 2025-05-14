#include <memory>
#include <set>
#include <string_view>

#include <uci/command.hpp>
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

std::unique_ptr<Command> Parser::Info() {
  std::unique_ptr<command::Info> command = std::make_unique<command::Info>();

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
    return command;
  }

  goto decide;
}

depth: {
  const Token &token = Consume(NUMBER, "Expect number after 'depth'.");

  command->depth = std::get<int>(token.literal);

  goto maybe_return;
}

seldepth: {
  const Token &token = Consume(NUMBER, "Expect number after 'seldepth'.");

  command->seldepth = std::get<int>(token.literal);

  goto maybe_return;
}

time: {
  const Token &token = Consume(NUMBER, "Expect number after 'time'.");

  command->time = std::get<int>(token.literal);

  goto maybe_return;
}

nodes: {
  const Token &token = Consume(NUMBER, "Expect number after 'nodes'.");

  command->nodes = std::get<int>(token.literal);

  goto maybe_return;
}

pv: {
  CONSUME_MOVES(command->pv, "Expect move after 'pv'.");

  goto maybe_return;
}

multipv: {
  const Token &token = Consume(NUMBER, "Expect number after 'multipv'.");

  command->multipv = std::get<int>(token.literal);

  goto maybe_return;
}

score: {
  const Token &token = Consume(WORD, "Expect word after 'score'.");
  std::string_view type = std::get<std::string_view>(token.literal);

  if (type == "cp") {
    const Token &token = Consume(NUMBER, "Expect number after 'cp'.");

    command->score.type = command::Info::Score::CP;
    command->score.value = std::get<int>(token.literal);
  } else if (type == "mate") {
    const Token &token = Consume(NUMBER, "Expect number after 'mate'.");

    command->score.type = command::Info::Score::MATE;
    command->score.value = std::get<int>(token.literal);
  } else if (type == "lowerbound") {
    command->score.type = command::Info::Score::LOWER_BOUND;
  } else if (type == "upperbound") {
    command->score.type = command::Info::Score::UPPER_BOUND;
  } else {
    throw Error(token,
                "Expect cp, mate, lowerbound or upperbound after 'score'.");
  }

  goto maybe_return;
}

  // TODO: check if it's valid move
currmove: {
  const Token &token = Consume(WORD, "Expect move after 'currmove'.");

  command->currmove = std::get<std::string_view>(token.literal);

  goto maybe_return;
}

currmovenumber: {
  const Token &token = Consume(NUMBER, "Expect number after 'currmovenumber'.");

  command->currmovenumber = std::get<int>(token.literal);

  if (command->currmovenumber < 1) {
    throw Error(token, "Expect currmovenumber to be greater than 0");
  }

  goto maybe_return;
}

hashfull: {
  const Token &token = Consume(NUMBER, "Expect number after 'hashfull'.");

  command->hashfull = std::get<int>(token.literal);

  goto maybe_return;
}

nps: {
  const Token &token = Consume(NUMBER, "Expect number after 'nps'.");

  command->nps = std::get<int>(token.literal);

  goto maybe_return;
}

tbhits: {
  const Token &token = Consume(NUMBER, "Expect number after 'tbhits'.");

  command->tbhits = std::get<int>(token.literal);

  goto maybe_return;
}

sbhits: {
  const Token &token = Consume(NUMBER, "Expect number after 'sbhits'.");

  command->sbhits = std::get<int>(token.literal);

  goto maybe_return;
}

cpuload: {
  const Token &token = Consume(NUMBER, "Expect number after 'cpuload'.");

  command->cpuload = std::get<int>(token.literal);

  goto maybe_return;
}

string: {
  const Token &token = Advance();

  // INFO: as per spec, rest of the line is considered the string value
  current_ = tokens_.size();
  command->string = token.lexeme.data();
}

refutation: {
  CONSUME_MOVES(command->refutation, "Expect move after 'refutation'.");

  goto maybe_return;
}

currline: {
  const Token &token = Consume(NUMBER, "Expect number after 'currline'.");

  command->currline.cpunr = std::get<int>(token.literal);

  if (command->currline.cpunr < 1) {
    throw Error(token, "Expect currline.cpunr to be greater than 0");
  }

  CONSUME_MOVES(command->currline.moves, "Expect move after cpu number.");

  goto maybe_return;
}

  return command;
}
