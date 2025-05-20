#ifndef TUI_SCREEN_ANALYZE_HPP
#define TUI_SCREEN_ANALYZE_HPP

#include <array>
#include <condition_variable>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include <ftxui/component/component_base.hpp>
#include <ftxui/component/event.hpp>
#include <tui/config.hpp>
#include <tui/theme.hpp>
#include <uci/uci.hpp>

using namespace uci;

namespace tui {
namespace screen {

class Analyze : public ftxui::ComponentBase, public UI {
public:
  Analyze(const Theme &theme);
  ~Analyze();

  bool OnEvent(ftxui::Event) override;
  ftxui::Element OnRender() override;

private:
  const Theme &theme_;

  std::string fen_;
  std::string pgn_;

  Engine engine_;
  std::mutex mutex_;
  std::thread thread_;
  std::condition_variable cv_;

  bool engine_is_uci_compatible_ = false;
  bool engine_is_ready_ = false;
  bool running_ = false;

  bool show_engine_settings_ = false;
  EngineOptions engine_options_;

  struct PV {
    int id = 0;
    float value;
    int depth;
    int nps = 0;
    command::Info::Score score;
    std::vector<std::string> moves;

    ftxui::Element Render() const;
  };

  std::array<PV, 256> pvs_;

  void Handle(command::Input *) override;
  void Handle(command::ID *) override;
  void Handle(command::BestMove *) override;
  void Handle(command::CopyProtection *) override;
  void Handle(command::Registration *) override;
  void Handle(command::Info *) override;
  void Handle(command::Option *) override;

  void InitEngineLoop();
  void OnRunSwitchChange();
  ftxui::Component MakeContainer();
};

} // namespace screen
} // namespace tui

#endif
