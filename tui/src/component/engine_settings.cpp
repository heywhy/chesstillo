#include <cstdint>
#include <format>
#include <functional>
#include <string>
#include <string_view>

#include <ftxui/component/component.hpp>

#include "uci/types.hpp"

#include "tui/color.hpp"
#include "tui/component/engine_settings.hpp"
#include "tui/config.hpp"
#include "tui/utils.hpp"

namespace tui {
namespace component {

EngineSettings::EngineSettings(const EngineOptions &options, OnChange on_change)
    : options_(const_cast<EngineOptions &>(options)), on_change_(on_change) {
  Add(ftxui::Container::Vertical({}));
}

ftxui::Element EngineSettings::OnRender() {
  ftxui::Elements settings;

  if (registry_.contains("Threads")) {
    settings.push_back(registry_["Threads"]->Render());
    settings.push_back(ftxui::separatorEmpty());
  }

  if (registry_.contains("MultiPV")) {
    settings.push_back(registry_["MultiPV"]->Render());
    settings.push_back(ftxui::separatorEmpty());
  }

  if (registry_.contains("Hash")) {
    settings.push_back(registry_["Hash"]->Render());
  }

  return ftxui::vbox(settings) | ftxui::color(color::gray400) |
         ftxui::size(ftxui::WIDTH, ftxui::EQUAL, 60);
}

void EngineSettings::Refresh() {
  DetachAllChildren();

  ftxui::Components components;

  if (options_.contains("Threads")) {
    components.push_back(Make("Threads", options_["Threads"]));
    registry_["Threads"] = components.back().get();
  }

  if (options_.contains("MultiPV")) {
    components.push_back(Make("MultiPV", options_["MultiPV"]));
    registry_["MultiPV"] = components.back().get();
  }

  if (options_.contains("Hash")) {
    components.push_back(Make("Hash", options_["Hash"]));
    registry_["Hash"] = components.back().get();
  }

  Add(ftxui::Container::Vertical(components));
}

ftxui::Component EngineSettings::Make(const std::string_view label,
                                      const EngineOption &option) {
  ftxui::Component component;
  EngineOption &e_option = const_cast<EngineOption &>(option);

  switch (option.type) {
    case uci::OptionType::CHECK:
      component = tui::Make<EngineSettings::Check>(label, e_option, on_change_);
      break;

    case uci::OptionType::SPIN:
      component = tui::Make<EngineSettings::Spin>(label, e_option, on_change_);
      break;

    case uci::OptionType::COMBO:
      component = tui::Make<EngineSettings::Combo>(label, e_option, on_change_);
      break;

      // case uci::BUTTON:
      //   component = tui::Make<EngineSettings::Button>(label, e_option);
      //   break;

    case uci::OptionType::STRING:
      component =
          tui::Make<EngineSettings::String>(label, e_option, on_change_);
      break;

    default:
      break;
  }

  return component;
}

EngineSettings::Check::Check(const std::string_view &label,
                             EngineOption &option, OnChange &on_change)
    : Option(label, option, std::get<bool>(option.value)) {
  ftxui::CheckboxOption opts = ftxui::CheckboxOption::Simple();

  opts.label = label.data();
  opts.checked = value_;
  opts.on_change = std::bind(on_change, &option_);

  Add(ftxui::Checkbox(opts));
}

EngineSettings::Spin::Spin(const std::string_view &label, EngineOption &option,
                           OnChange &on_change)
    : Option(label, option, std::get<std::int64_t>(option.value)) {
  ftxui::SliderOption<std::int64_t> opts;

  opts.min = option_.min;
  opts.max = option_.max;
  opts.increment = 1;

  opts.value = value_;
  opts.on_change = std::bind(on_change, &option_);

  Add(ftxui::Slider(opts));
}

ftxui::Element EngineSettings::Spin::OnRender() {
  auto element =
      ftxui::hbox({
          ftxui::text(label_.data()) | ftxui::vcenter,
          ftxui::separatorEmpty(),
          ftxui::hbox({
              ftxui::text("["),
              ftxui::dbox(
                  {ComponentBase::Render() | ftxui::underlined,
                   ftxui::text(std::format("{}", *value_)) | ftxui::center}) |
                  ftxui::xflex_grow,
              ftxui::text("]"),
          }) | ftxui::xflex,
      }) |
      ftxui::xflex;

  return element;
}

EngineSettings::Combo::Combo(const std::string_view &label,
                             EngineOption &option, OnChange &on_change)
    : Option(label, option), selected_(0) {
  *value_ = std::get<std::string>(option.value);

  ftxui::DropdownOption opts;

  opts.radiobox.entries = option.vars;
  opts.radiobox.selected = &selected_;
  opts.radiobox.on_change = [this, on_change] {
    *value_ = option_.vars[selected_];

    on_change(&option_);
  };

  Add(ftxui::Dropdown(opts));
}

EngineSettings::Button::Button(const std::string_view &label,
                               EngineOption &option,
                               std::function<void()> on_click)
    : Option(label, option) {
  ftxui::ButtonOption opts = ftxui::ButtonOption::Simple();

  opts.label = label_.data();
  opts.on_click = on_click;

  Add(ftxui::Button(opts));
}

EngineSettings::String::String(const std::string_view &label,
                               EngineOption &option, OnChange &on_change)
    : Option(label, option, std::get<std::string>(option.value)) {
  ftxui::InputOption opts = ftxui::InputOption::Default();

  opts.content = value_;
  opts.placeholder = label_.data();
  opts.cursor_position = value_ ? value_->size() : 0;
  opts.multiline = false;

  opts.on_change = std::bind(on_change, &option_);

  Add(ftxui::Input(opts));
}

ftxui::Element EngineSettings::String::OnRender() {
  return ftxui::hbox({ftxui::text(label_.data()), ftxui::separatorEmpty(),
                      ChildAt(0)->Render() | ftxui::underlined});
}

}  // namespace component
}  // namespace tui
