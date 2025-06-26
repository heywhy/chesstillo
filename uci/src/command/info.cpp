#include <cstdint>
#include <memory>
#include <set>
#include <string>
#include <string_view>

#include <uci/command.hpp>
#include <uci/parser.hpp>
#include <uci/types.hpp>

namespace uci {
#define IS_ATTR(token) \
  token.type ==        \
      TokenType::WORD &&M.contains(std::get<std::string_view>(token.literal))

// TODO: check if the token is a valid move string or abort
#define CONSUME_MOVES(list, error)                                   \
  while (!IsAtEnd()) {                                               \
    if (IS_ATTR(Peek())) {                                           \
      goto decide;                                                   \
    }                                                                \
    const Token &token = Consume(TokenType::WORD, error);            \
    const auto &literal = std::get<std::string_view>(token.literal); \
    list.emplace_back(literal);                                      \
  }

std::unique_ptr<Command> Parser::Info() {
  std::unique_ptr<command::Info> command = std::make_unique<command::Info>();

  static std::set<std::string_view> M = {
      "depth",      "seldepth", "time",     "nodes",          "pv",
      "multipv",    "score",    "currmove", "currmovenumber", "hashfull",
      "nps",        "tbhits",   "sbhits",   "cpuload",        "string",
      "refutation", "currline"};

decide: {
  const auto &token = Consume(TokenType::WORD, "Unexpected token.");
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
  const Token &token =
      Consume(TokenType::NUMBER, "Expect number after 'depth'.");

  command->depth = std::get<std::int64_t>(token.literal);

  goto maybe_return;
}

seldepth: {
  const Token &token =
      Consume(TokenType::NUMBER, "Expect number after 'seldepth'.");

  command->seldepth = std::get<std::int64_t>(token.literal);

  goto maybe_return;
}

time: {
  const Token &token =
      Consume(TokenType::NUMBER, "Expect number after 'time'.");

  command->time = std::get<std::int64_t>(token.literal);

  goto maybe_return;
}

nodes: {
  const Token &token =
      Consume(TokenType::NUMBER, "Expect number after 'nodes'.");

  command->nodes = std::get<std::int64_t>(token.literal);

  goto maybe_return;
}

pv: {
  CONSUME_MOVES(command->pv, "Expect move after 'pv'.");

  goto maybe_return;
}

multipv: {
  const Token &token =
      Consume(TokenType::NUMBER, "Expect number after 'multipv'.");

  command->multipv = std::get<std::int64_t>(token.literal);

  goto maybe_return;
}

score: {
  const Token &token = Consume(TokenType::WORD, "Expect word after 'score'.");
  std::string_view type = std::get<std::string_view>(token.literal);

  command->score = new command::Info::Score;

  if (type == "cp") {
    const Token &token =
        Consume(TokenType::NUMBER, "Expect number after 'cp'.");

    command->score->type = command::Info::Score::CP;
    command->score->value = std::get<std::int64_t>(token.literal);
  } else if (type == "mate") {
    const Token &token =
        Consume(TokenType::NUMBER, "Expect number after 'mate'.");

    command->score->type = command::Info::Score::MATE;
    command->score->value = std::get<std::int64_t>(token.literal);
  }

  if (Check(TokenType::WORD)) {
    const auto &literal = std::get<std::string_view>(Peek().literal);

    if (literal == "lowerbound") {
      const auto &token = Consume(TokenType::WORD);

      command->score->lowerbound = true;
    } else if (literal == "upperbound") {
      const auto &token = Consume(TokenType::WORD);

      command->score->upperbound = true;
    }
  }

  goto maybe_return;
}

  // TODO: check if it's valid move
currmove: {
  const Token &token =
      Consume(TokenType::WORD, "Expect move after 'currmove'.");

  command->currmove = std::get<std::string_view>(token.literal);

  goto maybe_return;
}

currmovenumber: {
  const Token &token =
      Consume(TokenType::NUMBER, "Expect number after 'currmovenumber'.");

  command->currmovenumber = std::get<std::int64_t>(token.literal);

  if (command->currmovenumber < 1) {
    throw Error(token, "Expect currmovenumber to be greater than 0");
  }

  goto maybe_return;
}

hashfull: {
  const Token &token =
      Consume(TokenType::NUMBER, "Expect number after 'hashfull'.");

  command->hashfull = std::get<std::int64_t>(token.literal);

  goto maybe_return;
}

nps: {
  const Token &token = Consume(TokenType::NUMBER, "Expect number after 'nps'.");

  command->nps = std::get<std::int64_t>(token.literal);

  goto maybe_return;
}

tbhits: {
  const Token &token =
      Consume(TokenType::NUMBER, "Expect number after 'tbhits'.");

  command->tbhits = std::get<std::int64_t>(token.literal);

  goto maybe_return;
}

sbhits: {
  const Token &token =
      Consume(TokenType::NUMBER, "Expect number after 'sbhits'.");

  command->sbhits = std::get<std::int64_t>(token.literal);

  goto maybe_return;
}

cpuload: {
  const Token &token =
      Consume(TokenType::NUMBER, "Expect number after 'cpuload'.");

  command->cpuload = std::get<std::int64_t>(token.literal);

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
  const Token &token =
      Consume(TokenType::NUMBER, "Expect number after 'currline'.");

  command->currline = new command::Info::Currline;

  command->currline->cpunr = std::get<std::int64_t>(token.literal);

  if (command->currline->cpunr < 1) {
    throw Error(token, "Expect currline.cpunr to be greater than 0");
  }

  CONSUME_MOVES(command->currline->moves, "Expect move after cpu number.");

  goto maybe_return;
}

  return command;
}

std::string command::Info::ToString() const {
  std::string str("info ");

  if (depth > 0) {
    str.append(" depth ").append(std::to_string(depth));
  }

  if (seldepth > 0) {
    str.append(" seldepth ").append(std::to_string(seldepth));
  }

  if (time > 0) {
    str.append(" time ").append(std::to_string(time));
  }

  if (nodes > 0) {
    str.append(" nodes ").append(std::to_string(nodes));
  }

  if (multipv > 0) {
    str.append(" multipv ").append(std::to_string(multipv));
  }

  if (currmovenumber > 0) {
    str.append(" currmovenumber ").append(std::to_string(currmovenumber));
  }

  if (hashfull > 0) {
    str.append(" hashfull ").append(std::to_string(hashfull));
  }

  if (nps > 0) {
    str.append(" nps ").append(std::to_string(nps));
  }

  if (tbhits > 0) {
    str.append(" tbhits ").append(std::to_string(tbhits));
  }

  if (sbhits > 0) {
    str.append(" sbhits ").append(std::to_string(sbhits));
  }

  if (cpuload > 0) {
    str.append(" cpuload ").append(std::to_string(cpuload));
  }

  if (!currmove.empty()) {
    str.append(" currmove ").append(currmove);
  }

  if (!string.empty()) {
    str.append(" string ").append(string);
  }

  if (!pv.empty()) {
    str.append(" pv");
  }

  for (const auto &move : pv) {
    str.append(" ").append(move);
  }

  if (!refutation.empty()) {
    str.append(" refutation");
  }

  for (const auto &move : refutation) {
    str.append(" ").append(move);
  }

  if (score != nullptr) {
    str.append(" score ").append(score->ToString());
  }

  if (currline != nullptr) {
    str.append(" currline ").append(currline->ToString());
  }

  return str;
}

std::string command::Info::Score::ToString() {
  std::string str;

  switch (type) {
    case CP:
      str.append("cp ").append(std::to_string(value));
      break;

    case MATE:
      str.append("mate ").append(std::to_string(value));
      break;
  }

  if (lowerbound) {
    str.append(" lowerbound");
  } else if (upperbound) {
    str.append(" upperbound");
  }

  return str;
}

std::string command::Info::Currline::ToString() {
  std::string str;

  if (cpunr > 1) {
    str.append(std::to_string(cpunr));
  }

  for (const auto &move : moves) {
    str.append(" ").append(move);
  }

  return str;
}
}  // namespace uci
