#include <memory>
#include <string_view>

#include <uci/command.hpp>
#include <uci/parser.hpp>
#include <uci/types.hpp>

namespace uci {
std::unique_ptr<Command> Parser::CopyProtection() {
  const auto &token =
      Consume(TokenType::WORD, "Expected checking, ok or error.");
  const auto &literal = std::get<std::string_view>(token.literal);

  if (!kStatus.contains(literal)) {
    throw Error(token, "Unexpected token.");
  }

  return std::make_unique<command::CopyProtection>(kStatus[literal]);
}

std::string command::CopyProtection::ToString() const {
  std::string str("copyprotection ");

  switch (status) {
    case Status::CHECKING:
      str.append("checking");
      break;

    case Status::OK:
      str.append("ok");
      break;

    case Status::ERROR:
      str.append("error");
      break;
  }

  return str;
}
}  // namespace uci
