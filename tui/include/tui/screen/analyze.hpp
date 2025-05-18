#ifndef TUI_SCREEN_ANALYZE_HPP
#define TUI_SCREEN_ANALYZE_HPP

#include <string>

#include <ftxui/component/component_base.hpp>
#include <ftxui/component/event.hpp>
#include <tui/theme.hpp>
#include <uci/uci.hpp>

using namespace uci;

namespace tui {
namespace screen {

class Analyze : public ftxui::ComponentBase, public UI {
public:
  Analyze(const Theme &theme);

  bool OnEvent(ftxui::Event) override;
  ftxui::Element OnRender() override;

private:
  const Theme &theme_;

  std::string fen_;
  std::string pgn_;

  Engine engine_;

  void Handle(command::Input *) override;
  void Handle(command::ID *) override;
  void Handle(command::BestMove *) override;
  void Handle(command::CopyProtection *) override;
  void Handle(command::Registration *) override;
  void Handle(command::Info *) override;
  void Handle(command::Option *) override;

  ftxui::Component MakeContainer();
};

} // namespace screen
} // namespace tui

#endif
