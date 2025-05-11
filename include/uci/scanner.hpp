#ifndef UCI_SCANNER_HPP
#define UCI_SCANNER_HPP

#include <string_view>

#include <uci/types.hpp>

#define TOKENIZE(output, input)                                                \
  {                                                                            \
    uci::Scanner scanner(input);                                               \
    output = std::move(scanner.Scan());                                        \
  }

namespace uci {
class Scanner {
public:
  Scanner(const std::string_view &input);

  Tokens &Scan();

private:
  const std::string_view &input_;

  Tokens tokens_;

  int line_;
  int start_;
  std::string_view::size_type current_;

  char Peek();
  char Advance();
  void ScanToken();

  void Word();
  void Digit();

  bool IsAtEnd();

  void AddToken(uci::TokenType type);
  void AddToken(uci::TokenType type, auto c);
};
} // namespace uci

#endif
