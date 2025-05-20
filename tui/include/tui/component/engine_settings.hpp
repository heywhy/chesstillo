#ifndef TUI_COMPONENT_ENGINE_SETTINGS_HPP
#define TUI_COMPONENT_ENGINE_SETTINGS_HPP

#include <ftxui/component/component_base.hpp>
#include <tui/config.hpp>

namespace tui {
namespace component {

class EngineSettings : public ftxui::ComponentBase {
public:
  EngineSettings(const EngineOptions &options);

  void Refresh();
  ftxui::Element OnRender() override;

private:
  const EngineOptions &options_;
};

} // namespace component
} // namespace tui

#endif
