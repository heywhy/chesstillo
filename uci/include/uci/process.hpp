#ifndef UCI_PROCESS_HPP
#define UCI_PROCESS_HPP

#include <uci/command.hpp>

namespace uci {
class Link;

struct Process {
  Link *link;

  virtual void Handle(command::Input *) = 0;
  virtual void Handle(command::Debug *) = 0;
  virtual void Handle(command::SetOption *) = 0;
  virtual void Handle(command::Register *) = 0;
  virtual void Handle(command::Position *) = 0;
  virtual void Handle(command::Go *) = 0;
};
}  // namespace uci

#endif
