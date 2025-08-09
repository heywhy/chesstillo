#include <cstddef>
#include <memory>
#include <string>
#include <string_view>
#include <thread>
#include <utility>

#include <boost/process.hpp>
#include <boost/process/v2/environment.hpp>
#include <boost/system/system_error.hpp>
#include <boost/throw_exception.hpp>

#include "uci/command.hpp"
#include "uci/engine.hpp"
#include "uci/parser.hpp"
#include "uci/scanner.hpp"
#include "uci/types.hpp"

namespace asio = boost::asio;
namespace bsystem = boost::system;

namespace uci {
Engine::Engine(const std::string_view &path, UI *ui)
    : ui_(ui),
      stop_(false),
      instance_(context_.get_executor(), path, {}),
      thread_(&Engine::Loop, this) {}

Engine::~Engine() {
  if (instance_.running()) {
    instance_.terminate();
  }

  stop_ = true;

  thread_.join();
}

void Engine::Send(uci::Command &command) { command.Accept(*this); }

void Engine::Send(uci::Command &&temp) {
  using T = decltype(temp);

  T command = std::move(temp);

  Send(command);
}

void Engine::Loop() {
  try {
    std::string output;

    while (!stop_) {
      Tokens tokens;
      std::unique_ptr<Command> command(nullptr);

      std::size_t size = asio::read_until(
          instance_, asio::dynamic_string_buffer(output), "\n");

      std::string line = output.substr(0, size);

      output.erase(0, size);

      TOKENIZE(tokens, line);

      if (tokens.empty()) {
        continue;
      }

      try {
        PARSE(command, tokens)

        command->Accept(*this);
      } catch (ParseError &e) {
        // INFO: write both line read and error to log?
      }
    }
  } catch (boost::wrapexcept<bsystem::system_error> &e) {
  }
}

void Engine::VisitInput(command::Input *command) {
  if (command->IsFeedback()) {
    ui_->Handle(command);
  } else {
    WriteToProcess(command->ToString());
  }
}

void Engine::VisitDebug(command::Debug *command) {
  WriteToProcess(command->ToString());
}

void Engine::VisitPosition(command::Position *command) {
  WriteToProcess(command->ToString());
}

void Engine::VisitGo(command::Go *command) {
  WriteToProcess(command->ToString());
}

void Engine::VisitSetOption(command::SetOption *command) {
  WriteToProcess(command->ToString());
}

void Engine::VisitRegister(command::Register *command) {
  WriteToProcess(command->ToString());
}

void Engine::VisitID(command::ID *command) { ui_->Handle(command); }

void Engine::VisitBestMove(command::BestMove *command) { ui_->Handle(command); }

void Engine::VisitCopyProtection(command::CopyProtection *command) {
  ui_->Handle(command);
}

void Engine::VisitRegistration(command::Registration *command) {
  ui_->Handle(command);
}

void Engine::VisitInfo(command::Info *command) { ui_->Handle(command); }

void Engine::VisitOption(command::Option *command) { ui_->Handle(command); }

void Engine::WriteToProcess(std::string &&line) {
  line.append("\n");

  asio::write(instance_, asio::buffer(line));
}
}  // namespace uci
