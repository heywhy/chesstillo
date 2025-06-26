#ifndef UCI_LINK_HPP
#define UCI_LINK_HPP

#include <iostream>
#include <string>

#include <uci/command.hpp>
#include <uci/process.hpp>

namespace uci {
class Link : Visitor {
 public:
  Link(std::istream &in, std::ostream &out, Process *);

  void Loop();
  void Send(Command &command);
  void Send(Command &&command);

 private:
  std::istream &in_;
  std::ostream &out_;
  Process *process_;

  bool quit_;

  void WriteToUI(std::string &&line);

  void VisitInput(command::Input *) override;
  void VisitDebug(command::Debug *) override;
  void VisitPosition(command::Position *) override;
  void VisitGo(command::Go *) override;
  void VisitSetOption(command::SetOption *) override;
  void VisitRegister(command::Register *) override;
  void VisitID(command::ID *) override;
  void VisitBestMove(command::BestMove *) override;
  void VisitCopyProtection(command::CopyProtection *) override;
  void VisitRegistration(command::Registration *) override;
  void VisitInfo(command::Info *) override;
  void VisitOption(command::Option *) override;
};
}  // namespace uci

#endif
