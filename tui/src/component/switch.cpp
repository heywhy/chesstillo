#include <functional>
#include <string>
#include <vector>

#include <tui/component/switch.hpp>

namespace tui {
namespace component {

Switch::Switch(bool &on) : Switch({"Off", "On"}, on, [] {}) {}
Switch::Switch(bool &on, OnChange on_change)
    : Switch({"Off", "On"}, on, on_change) {}

Switch::Switch(const Labels &labels, bool &on, const OnChange on_change)
    : labels(labels), selected_(on ? 1 : 0), on_change_(on_change) {
  ftxui::MenuOption option = ftxui::MenuOption::Toggle();
  std::vector<std::string> entries(labels.begin(), labels.end());

  option.on_change = [this, &on] {
    on = selected_ == 1;
    on_change_();
  };

  Add(ftxui::Menu(std::move(entries), &selected_, option));
}

}  // namespace component
}  // namespace tui
