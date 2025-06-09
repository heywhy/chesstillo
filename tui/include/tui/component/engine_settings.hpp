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
  EngineSettings(const EngineOptions &options);

  void Refresh();
  ftxui::Element OnRender() override;

  template <typename T>
  class Option : public ftxui::ComponentBase {
   public:
    Option(const std::string_view &label, EngineOption &option, T &value)
        : label_(label), option_(option), value_(&value) {}

    Option(const std::string_view &label, EngineOption &option)
        : label_(label), option_(option), value_(nullptr) {}

   protected:
    const std::string_view label_;
    EngineOption &option_;
    T *value_;
  };

  class Check : public Option<bool> {
   public:
    Check(const std::string_view &label, EngineOption &option);
  };

  class Spin : public Option<std::int64_t> {
   public:
    Spin(const std::string_view &label, EngineOption &option);

    ftxui::Element OnRender() override;

   private:
    ftxui::Box box_;
  };

  class Combo : public Option<std::string> {
   public:
    Combo(const std::string_view &label, EngineOption &option);

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
    String(const std::string_view &label, EngineOption &option);

    ftxui::Element OnRender() override;
  };

 private:
  EngineOptions &options_;
  std::map<std::string, ftxui::ComponentBase *> registry_;

  ftxui::Component Make(const std::string_view label,
                        const EngineOption &option);
};

}  // namespace component
}  // namespace tui

#endif
