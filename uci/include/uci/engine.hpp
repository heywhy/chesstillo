#ifndef UCI_PROCESS_HPP
#define UCI_PROCESS_HPP

#include <mutex>
#include <string>
#include <string_view>
#include <thread>

#include <boost/asio.hpp>
#include <boost/process.hpp>
#include <uci/command.hpp>
#include <uci/ui.hpp>

namespace asio = boost::asio;
namespace process = boost::process;

namespace uci {
class Engine : Visitor {
public:
  Engine(const std::string_view &path, UI *ui);
  ~Engine();

  bool IsRunning();
  void Send(uci::Command &command);
  void Send(uci::Command &&command);

protected:
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

private:
  UI *ui_;
  bool stop_;

  asio::io_context context_;
  process::popen instance_;

  std::mutex mutex_;
  std::thread thread_;

  void Loop();
  void WriteToProcess(std::string &&line);
};
} // namespace uci

#endif
