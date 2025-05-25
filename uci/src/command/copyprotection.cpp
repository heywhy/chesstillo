#include <memory>
#include <string_view>

#include <uci/command.hpp>
#include <uci/parser.hpp>
#include <uci/types.hpp>

using namespace uci;

std::unique_ptr<Command> Parser::CopyProtection() {
  const auto &token = Consume(WORD, "Expected checking, ok or error.");
  const auto &literal = std::get<std::string_view>(token.literal);

  if (!kStatus.contains(literal)) {
    throw Error(token, "Unexpected token.");
  }

  return std::make_unique<command::CopyProtection>(kStatus[literal]);
}

std::string command::CopyProtection::ToString() const {
  std::string str("copyprotection ");

  switch (status) {
    case CHECKING:
      str.append("checking");
      break;

    case OK:
      str.append("ok");
      break;

    case ERROR:
      str.append("error");
      break;
  }

  return str;
}
