#include <format>
#include <istream>
#include <memory>
#include <ostream>
#include <string>
#include <utility>

#include <uci/command.hpp>
#include <uci/link.hpp>
#include <uci/parser.hpp>
#include <uci/scanner.hpp>
#include <uci/types.hpp>

namespace uci {
constexpr const char *kUnknownMsg = "Unknown command '{}'.";

Link::Link(std::istream &in, std::ostream &out)
    : in_(in), out_(out), quit_(false) {}

void Link::Send(Command &command) { command.Accept(*this); }

void Link::Send(Command &&temp) {
  using T = decltype(temp);

  T command = std::move(temp);

  Send(command);
}

void Link::Loop() {
  while (!quit_ && !in_.eof()) {
    Tokens tokens;
    std::string line;
    command::Input *input;
    std::unique_ptr<Command> command(nullptr);

    std::getline(in_, line);

    TOKENIZE(tokens, line);

    if (tokens.empty()) {
      continue;
    }

    if (IsFeedback(tokens[0].type)) {
      WriteToUI(std::format(kUnknownMsg, line));
      continue;
    }

    PARSE(command, tokens);

    if (command.get() == nullptr) {
      if (!line.empty()) {
        WriteToUI(std::format(kUnknownMsg, line));
      }

      continue;
    }

    command->Accept(*this);
  }
}

void Link::VisitInput(command::Input *command) {
  switch (command->type) {
    case TokenType::QUIT:
      quit_ = true;
      break;

    case TokenType::UCI:
    case TokenType::IS_READY:
    case TokenType::UCI_NEW_GAME:
    case TokenType::STOP:
    case TokenType::PONDER_HIT:
      Handle(command);
      break;

    case TokenType::UCI_OK:
    case TokenType::READY_OK:
      WriteToUI(command->ToString());
      break;

    default:
      WriteToUI(std::format(kUnknownMsg, command->ToString()));
      break;
  }
}

void Link::VisitDebug(command::Debug *command) { Handle(command); }

void Link::VisitPosition(command::Position *command) { Handle(command); }

void Link::VisitGo(command::Go *command) { Handle(command); }

void Link::VisitSetOption(command::SetOption *command) { Handle(command); }

void Link::VisitRegister(command::Register *command) { Handle(command); }

void Link::VisitID(command::ID *command) { WriteToUI(command->ToString()); }

void Link::VisitBestMove(command::BestMove *command) {
  WriteToUI(command->ToString());
}

void Link::VisitCopyProtection(command::CopyProtection *command) {
  WriteToUI(command->ToString());
}

void Link::VisitRegistration(command::Registration *command) {
  WriteToUI(command->ToString());
}

void Link::VisitInfo(command::Info *command) { WriteToUI(command->ToString()); }

void Link::VisitOption(command::Option *command) {
  WriteToUI(command->ToString());
}

void Link::WriteToUI(std::string &&line) { out_ << line << std::endl; }

}  // namespace uci
