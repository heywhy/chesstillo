#ifndef UCI_PARSER_HPP
#define UCI_PARSER_HPP

#include <memory>
#include <stdexcept>

#include <uci/command.hpp>
#include <uci/types.hpp>

#define PARSE(expr, tokens)                                                    \
  {                                                                            \
    uci::Parser parser(tokens);                                                \
    auto uq = parser.Parse();                                                  \
    decltype(expr)::pointer ptr =                                              \
        static_cast<decltype(expr)::pointer>(uq.release());                    \
    expr.reset(ptr);                                                           \
  }

namespace uci {
class ParseError : public std::runtime_error {
public:
  ParseError(const std::string_view &message)
      : std::runtime_error(message.data()) {}
};

class Parser {
public:
  Parser(const Tokens &tokens);

  std::unique_ptr<Command> Parse();

private:
  const Tokens &tokens_;
  Tokens::size_type current_;

  Tokens::const_reference &Peek();

  Tokens::const_reference Advance();
  Tokens::const_reference Previous();

  static void Report(int line, const std::string_view &where,
                     const std::string_view &message);

  bool IsAtEnd();
  bool Check(TokenType type);
  bool Match(TokenType type);
  Tokens::const_reference
  Consume(TokenType type,
          const std::string_view &message = "Unexpected token.");

  ParseError Error(const Token &token,
                   const std::string_view &message = "Unexpected token.");

  // GUI to Engine
  std::unique_ptr<Command> Position();
  std::unique_ptr<Command> Go();
  std::unique_ptr<Command> SetOption();
  std::unique_ptr<Command> Register();

  // Engine to GUI
  std::unique_ptr<Command> ID();
  std::unique_ptr<Command> BestMove();
  std::unique_ptr<Command> CopyProtection();
  std::unique_ptr<Command> Registration();
  std::unique_ptr<Command> Info();
  std::unique_ptr<Command> Option();
};
} // namespace uci

#endif
