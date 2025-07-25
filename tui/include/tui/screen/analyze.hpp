#ifndef TUI_SCREEN_ANALYZE_HPP
#define TUI_SCREEN_ANALYZE_HPP

#include <array>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include <engine/engine.hpp>
#include <ftxui/component/component_base.hpp>
#include <uci/uci.hpp>

#include <tui/components.hpp>
#include <tui/config.hpp>
#include <tui/contracts.hpp>
#include <tui/theme.hpp>
#include <tui/types.hpp>

namespace tui {
namespace screen {

class Analyze : public component::View {
 public:
  Analyze(const Theme &theme);

 protected:
  void BindKeymaps() override;

 private:
  EngineOptions engine_options_;
};

namespace analyze {
class Main : public ftxui::ComponentBase, public uci::UI {
 public:
  Main(const Theme &theme, component::View *view,
       EngineOptions &engine_options);
  ~Main();

  bool OnEvent(ftxui::Event event) override;
  ftxui::Element OnRender() override;

 private:
  const Theme &theme_;
  component::View *const view_;
  EngineOptions &engine_options_;

  std::string fen_;
  std::string pgn_;
  component::Square *selected_;

  engine::Position position_;
  std::shared_ptr<component::Input> fen_input_;
  std::shared_ptr<component::Input> pgn_input_;
  std::shared_ptr<component::Chessboard> chessboard_;

  uci::Engine engine_;
  std::string engine_name_;
  std::string engine_author_;

  std::mutex mutex_;
  std::thread thread_;
  std::condition_variable cv_;

  bool running_ = false;

  EngineAttrs engine_attrs_ = 0;

  struct PV {
    unsigned int id = 0;
    float value;
    int depth;
    int nps = 0;
    std::vector<std::string> moves;
    engine::Position position;

    ftxui::Box box;
    bool show_all_moves = false;

    ftxui::Element Render();
    bool OnEvent(ftxui::Event &event);

    inline void Unset() {
      id = 0;
      moves.clear();
    }
  };

  std::array<PV, 256> pvs_;

  friend class tui::screen::Analyze;

  void Handle(uci::command::Input *) override;
  void Handle(uci::command::ID *) override;
  void Handle(uci::command::BestMove *) override;
  void Handle(uci::command::CopyProtection *) override;
  void Handle(uci::command::Registration *) override;
  void Handle(uci::command::Info *) override;
  void Handle(uci::command::Option *) override;

  void InitEngine();
  ftxui::Component MakeContainer();

  ftxui::Element RenderMoves();
  ftxui::Element RenderStatusBar();

  void Stop();
  void Start();
  void ClearHash();
  void ShowSettings();
  void ShowEngineInfo();

  void MaybeMove(component::Square *);

  void OnFenChange();
  void OnPgnChange();
  void UpdateBoard();
  void OnChange(const tui::EngineOption *);
  void ToggleSquare(component::Square *square);

  template <typename T>
  void SetAndSendOption(std::string id, T value) {
    auto &option = engine_options_[id];

    option.value = value;

    uci::command::SetOption command(
        uci::command::SetOption(id, std::get<T>(option.value)));

    engine_.Send(command);
  }
};

}  // namespace analyze
}  // namespace screen
}  // namespace tui

#endif
