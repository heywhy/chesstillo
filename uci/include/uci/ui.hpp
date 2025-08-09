#ifndef UCI_UI_HPP
#define UCI_UI_HPP

#include "command.hpp"

namespace uci {
struct UI {
  virtual void Handle(command::Input *) = 0;
  virtual void Handle(command::ID *) = 0;
  virtual void Handle(command::BestMove *) = 0;
  virtual void Handle(command::CopyProtection *) = 0;
  virtual void Handle(command::Registration *) = 0;
  virtual void Handle(command::Info *) = 0;
  virtual void Handle(command::Option *) = 0;
};
}  // namespace uci

#endif
