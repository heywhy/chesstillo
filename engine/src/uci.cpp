#include "uci/command.hpp"
#include "uci/link.hpp"

#include "engine/uci.hpp"

static command::ID kEngineAuthor(command::ID::Type::AUTHOR, "Rasheed Atanda");
static command::ID kEngineName(command::ID::Type::NAME, "Chesstillo 0.1");

namespace engine {
UCILink::UCILink(Position *position)
    : uci::Link(std::cin, std::cout), position_(position) {}

void UCILink::Handle(command::Input *command) {
  static uci::command::Input kUciOk("uciok");
  static uci::command::Input kReadyOk("readyok");

  switch (command->type) {
    case uci::TokenType::UCI:
      Send(kEngineName);
      Send(kEngineAuthor);
      // TODO: send supported options
      Send(kUciOk);
      break;

    case uci::TokenType::UCI_NEW_GAME:
      // TODO: clear all search data
      position_->Reset();
      break;

    case uci::TokenType::IS_READY:
      Send(kReadyOk);
      break;

    default:
      break;
  }
}

void UCILink::Handle(command::Debug *) {}
void UCILink::Handle(command::SetOption *) {}
void UCILink::Handle(command::Register *) {}

void UCILink::Handle(command::Position *command) {
  fen_ = command->input == "startpos" ? engine::kStartPos : command->input;

  // TODO: validate supplied fen by checking the state of the board
  // * make sure that both kings exist on the board
  // * make sure other piece(s) asides the kings exist on the board
  engine::Position::ApplyFen(position_, fen_);
}

void UCILink::Handle(command::Go *) {}
}  // namespace engine
