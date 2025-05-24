#include <cstdint>
#include <format>
#include <functional>
#include <string>
#include <string_view>

#include <ftxui/component/component.hpp>
#include <tui/component/engine_settings.hpp>
#include <tui/config.hpp>
#include <uci/uci.hpp>

namespace tui {
namespace component {

EngineSettings::EngineSettings(const EngineOptions &options)
    : options_(options) {
  Add(ftxui::Container::Vertical({}));
}

ftxui::Element EngineSettings::OnRender() {
  ftxui::Elements settings;

  auto container = ChildAt(0);
  std::size_t count = container->ChildCount();

  settings.push_back(ftxui::separatorEmpty());

  for (std::size_t i = 0; i < count; i++) {
    auto &child = container->ChildAt(i);

    settings.push_back(child->Render());

    if (child.get() != children_.end()->get()) {
      settings.push_back(ftxui::separatorEmpty());
    }
  }

  return ftxui::vbox(settings);
}

void EngineSettings::Refresh() {
  DetachAllChildren();

  ftxui::Components components;

  for (const auto &[id, option] : options_) {
    if (option.type == uci::BUTTON) {
      // if (option.type == uci::BUTTON || option.type == uci::CHECK) {
      continue;
    }

    components.push_back(Make(id, option));
  }

  Add(ftxui::Container::Vertical(components));
}

ftxui::Component EngineSettings::Make(const std::string_view label,
                                      const EngineOption &option) {
  ftxui::Component component;
  EngineOption &e_option = const_cast<EngineOption &>(option);

  switch (option.type) {
  case uci::CHECK:
    component = ftxui::Make<EngineSettings::Check>(label, e_option);
    break;

  case uci::SPIN:
    component = ftxui::Make<EngineSettings::Spin>(label, e_option);
    break;

  case uci::COMBO:
    component = ftxui::Make<EngineSettings::Combo>(label, e_option);
    break;

    // case uci::BUTTON:
    //   component = ftxui::Make<EngineSettings::Button>(label, e_option);
    //   break;

  case uci::STRING:
    component = ftxui::Make<EngineSettings::String>(label, e_option);
    break;

  default:
    break;
  }

  return component;
}

EngineSettings::Check::Check(const std::string_view &label,
                             EngineOption &option)
    : Option(label, option, std::get<bool>(option.value)) {
  ftxui::CheckboxOption opts = ftxui::CheckboxOption::Simple();

  opts.label = label.data();
  opts.checked = &value_;
  opts.on_change = [&]() { option.value = value_; };

  Add(ftxui::Checkbox(opts));
}

EngineSettings::Spin::Spin(const std::string_view &label, EngineOption &option)
    : Option(label, option, std::get<std::int64_t>(option.value)) {
  ftxui::SliderOption<std::int64_t> opts;

  opts.min = option.min;
  opts.max = option.max;
  opts.increment = 1;

  opts.value = &value_;
  opts.on_change = [&]() { option.value = value_; };

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
                   ftxui::text(std::format("{}", value_)) | ftxui::center}) |
                  ftxui::xflex_grow,
              ftxui::text("]"),
          }) | ftxui::xflex,
      }) |
      ftxui::xflex | ftxui::reflect(box_);

  return element;
}

EngineSettings::Combo::Combo(const std::string_view &label,
                             EngineOption &option)
    : Option(label, option) {
  value_ = std::get<std::string>(option.value);

  ftxui::DropdownOption opts;

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
                               EngineOption &option)
    : Option(label, option, std::get<std::string>(option.value)) {
  ftxui::InputOption opts = ftxui::InputOption::Default();

  opts.content = &value_;
  opts.placeholder = label_.data();
  opts.cursor_position = value_.size();
  opts.multiline = false;

  opts.on_change = [&]() { option.value = value_; };

  Add(ftxui::Input(opts));
}

ftxui::Element EngineSettings::String::OnRender() {
  return ftxui::hbox({ftxui::text(label_.data()), ftxui::separatorEmpty(),
                      ChildAt(0)->Render() | ftxui::underlined});
}

} // namespace component
} // namespace tui
