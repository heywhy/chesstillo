#include <iostream>
#include <memory>

#include <uci/expr.hpp>
#include <uci/parser.hpp>
#include <uci/types.hpp>

using namespace uci;

Parser::Parser(const Tokens &tokens) : tokens_(tokens), current_(0) {}

std::unique_ptr<Expr> Parser::Parse() {
  try {
    std::unique_ptr<Expr> expr;
    Tokens::const_reference token = Advance();

    switch (token.type) {
    case UCI:
    case IS_READY:
    case UCI_NEW_GAME:
    case STOP:
    case PONDER_HIT:
    case QUIT:
    case UCI_OK:
    case READY_OK:
      expr.reset(new expr::Command(token.lexeme));
      break;

    case POSITION:
      expr = Position();
      break;

    case GO:
      expr = Go();
      break;

    case SET_OPTION:
      expr = SetOption();
      break;

    case REGISTER:
      expr = Register();
      break;

    case uci::TokenType::ID:
      expr = ID();
      break;

    case BEST_MOVE:
      expr = BestMove();
      break;

    case COPY_PROTECTION:
      expr = CopyProtection();
      break;

    case REGISTRATION:
      expr = Registration();
      break;

    case INFO:
      expr = Info();
      break;

    case uci::TokenType::OPTION:
      expr = Option();
      break;

    default:
      expr = nullptr;
      break;
    }

    if (!IsAtEnd()) {
      throw Error(Peek(), "Unexpected token.");
    }

    return expr;
  } catch (ParseError &) {
    return nullptr;
  }
}

bool Parser::Match(TokenType type) {
  if (Check(type)) {
    Advance();
    return true;
  }

  return false;
}

Tokens::const_reference Parser::Consume(TokenType type,
                                        const std::string_view &message) {
  if (Check(type)) {
    return Advance();
  }

  throw Error(Peek(), message);
}

ParseError Parser::Error(const Token &token, const std::string_view &message) {
  std::string where(" at '");

  where += token.lexeme;
  where += "'";

  Report(token.line, where, message);

  return {message};
}

void Parser::Report(int line, const std::string_view &where,
                    const std::string_view &message) {
  std::cerr << "[line " << line << "] Error" << where << ": " << message
            << std::endl;
}

bool Parser::Check(TokenType type) {
  if (IsAtEnd()) {
    return false;
  }

  return Peek().type == type;
}

bool Parser::IsAtEnd() { return current_ >= tokens_.size(); }

Tokens::const_reference Parser::Advance() { return tokens_[current_++]; }

Tokens::const_reference Parser::Previous() { return tokens_[current_ - 1]; }

Tokens::const_reference Parser::Peek() { return tokens_[current_]; }
