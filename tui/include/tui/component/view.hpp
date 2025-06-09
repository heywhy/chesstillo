#ifndef TUI_COMPONENT_VIEW_HPP
#define TUI_COMPONENT_VIEW_HPP

#include <stack>

#include <ftxui/component/component_base.hpp>
#include <ftxui/component/event.hpp>

#include <tui/component/command_input.hpp>
#include <tui/component/modal_view.hpp>
#include <tui/mapping.hpp>

namespace tui {
namespace component {

class View : public ModalView {
 public:
  View(tui::Mode mode = tui::NORMAL);

  ftxui::Element OnRender() override;
  bool OnEvent(ftxui::Event event) override;
  void SetActiveChild(ftxui::ComponentBase *child) override;

  ftxui::Element RenderStatusBar(const KeyPairs &) const;

  void ShowModal(ftxui::Component component);
  void HideModal();

 protected:
  void BindKeymaps() override;

 private:
  bool show_modal_;
  bool show_prompt_;
  std::string prompt_value_;

  std::shared_ptr<CommandInput> prompt_;
  ftxui::Component modal_content_;

  std::stack<ftxui::ComponentBase *> focus_history_;

  void PopAndFocusTopComponent();
};

}  // namespace component
}  // namespace tui

#endif
