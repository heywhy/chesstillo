#ifndef ENGINE_UCI_HPP
#define ENGINE_UCI_HPP

#include <string>

#include <uci/uci.hpp>

#include <engine/position.hpp>

namespace command = uci::command;

namespace engine {
class UCILink : public uci::Link {
 public:
  UCILink(Position *position);

 protected:
  void Handle(command::Input *command) override;

  void Handle(command::Debug *) override;
  void Handle(command::SetOption *) override;
  void Handle(command::Register *) override;
  void Handle(command::Position *command) override;
  void Handle(command::Go *) override;

 private:
  engine::Position *position_;
  std::string fen_;
};
}  // namespace engine

#endif
