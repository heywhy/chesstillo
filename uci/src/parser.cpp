#include <iostream>
#include <memory>

#include <uci/command.hpp>
#include <uci/parser.hpp>
#include <uci/types.hpp>

namespace uci {
Parser::Parser(const Tokens &tokens) : tokens_(tokens), current_(0) {}

std::unique_ptr<Command> Parser::Parse() {
  try {
    std::unique_ptr<Command> command;
    Tokens::const_reference token = Advance();

    switch (token.type) {
      case TokenType::UCI:
      case TokenType::IS_READY:
      case TokenType::UCI_NEW_GAME:
      case TokenType::STOP:
      case TokenType::PONDER_HIT:
      case TokenType::QUIT:
      case TokenType::UCI_OK:
      case TokenType::READY_OK:
        command.reset(new command::Input(token.lexeme));
        break;

      case TokenType::DEBUG:
        command = Debug();
        break;

      case TokenType::POSITION:
        command = Position();
        break;

      case TokenType::GO:
        command = Go();
        break;

      case TokenType::SET_OPTION:
        command = SetOption();
        break;

      case TokenType::REGISTER:
        command = Register();
        break;

      case uci::TokenType::ID:
        command = ID();
        break;

      case TokenType::BEST_MOVE:
        command = BestMove();
        break;

      case TokenType::COPY_PROTECTION:
        command = CopyProtection();
        break;

      case TokenType::REGISTRATION:
        command = Registration();
        break;

      case TokenType::INFO:
        command = Info();
        break;

      case TokenType::OPTION:
        command = Option();
        break;

      default:
        command = nullptr;
        break;
    }

    if (!IsAtEnd()) {
      throw Error(Peek(), "Unexpected token.");
    }

    return command;
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

  return message;
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
}  // namespace uci
