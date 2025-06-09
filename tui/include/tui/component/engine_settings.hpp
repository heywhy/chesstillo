#ifndef TUI_COMPONENT_ENGINE_SETTINGS_HPP
#define TUI_COMPONENT_ENGINE_SETTINGS_HPP

#include <cstdint>
#include <functional>
#include <map>
#include <string_view>

#include <ftxui/component/component_base.hpp>

#include <tui/config.hpp>

namespace tui {
namespace component {

class EngineSettings : public ftxui::ComponentBase {
 public:
  using OnChange = std::function<void(const EngineOption *)>;

  EngineSettings(const EngineOptions &options, OnChange on_change);

  void Refresh();
  ftxui::Element OnRender() override;

  template <typename T>
  class Option : public ftxui::ComponentBase {
   public:
    Option(const std::string_view &label, EngineOption &option, T &value)
        : Option(label, option, &value) {}

    Option(const std::string_view &label, EngineOption &option)
        : Option(label, option, nullptr) {}

    Option(const std::string_view &label, EngineOption &option, T *value)
        : label_(label), option_(option), value_(value) {}

   protected:
    const std::string_view label_;
    EngineOption &option_;
    T *value_;
  };

  class Check : public Option<bool> {
   public:
    Check(const std::string_view &label, EngineOption &option,
          OnChange &on_change);
  };

  class Spin : public Option<std::int64_t> {
   public:
    Spin(const std::string_view &label, EngineOption &option,
         OnChange &on_change);

    ftxui::Element OnRender() override;
  };

  class Combo : public Option<std::string> {
   public:
    Combo(const std::string_view &label, EngineOption &option,
          OnChange &on_change);

   private:
    int selected_;
  };

  class Button : public Option<void *> {
   public:
    Button(const std::string_view &label, EngineOption &option,
           std::function<void()> on_click);
  };

  class String : public Option<std::string> {
   public:
    String(const std::string_view &label, EngineOption &option,
           OnChange &on_change);

    ftxui::Element OnRender() override;
  };

 private:
  EngineOptions &options_;
  OnChange on_change_;
  std::map<std::string, ftxui::ComponentBase *> registry_;

  ftxui::Component Make(const std::string_view label,
                        const EngineOption &option);
};

}  // namespace component
}  // namespace tui

#endif
