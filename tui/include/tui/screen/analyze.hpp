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
#include <uci/uci.hpp>

#include <tui/components.hpp>
#include <tui/config.hpp>
#include <tui/theme.hpp>

using namespace uci;

namespace tui {
namespace screen {

class Analyze : public ftxui::ComponentBase {
 public:
  Analyze(const Theme &theme);

  bool OnEvent(ftxui::Event) override;

 private:
  bool show_engine_settings_;
  EngineOptions engine_options_;
};

namespace analyze {
class Main : public ftxui::ComponentBase, public UI {
 public:
  Main(const Theme &theme, EngineOptions &engine_options,
       component::EngineSettings *);
  ~Main();

  ftxui::Element OnRender() override;

 private:
  const Theme &theme_;
  EngineOptions &engine_options_;
  component::EngineSettings *engine_settings_;

  std::string fen_;
  std::string pgn_;

  Engine engine_;
  std::mutex mutex_;
  std::thread thread_;
  std::condition_variable cv_;

  bool engine_is_uci_compatible_ = false;
  bool engine_is_ready_ = false;
  bool running_ = false;

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

}  // namespace analyze
}  // namespace screen
}  // namespace tui

#endif
