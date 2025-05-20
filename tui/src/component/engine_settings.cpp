#include <ftxui/dom/elements.hpp>
#include <tui/component/engine_settings.hpp>

namespace tui {
namespace component {

EngineSettings::EngineSettings(const EngineOptions &options)
    : options_(options) {}

ftxui::Element EngineSettings::OnRender() {
  ftxui::Elements settings;

  for (const auto &[id, _] : options_) {
    ftxui::Element setting = ftxui::hbox({ftxui::text(id)});

    settings.push_back(setting);
  }

  return ftxui::vbox(settings);
}

void EngineSettings::Refresh() { DetachAllChildren(); }

} // namespace component
} // namespace tui
