#ifndef TUI_COMPONENT_VIEW_HPP
#define TUI_COMPONENT_VIEW_HPP

#include <stack>

#include <ftxui/component/component_base.hpp>

#include "tui/component/command_input.hpp"
#include "tui/component/modal_view.hpp"
#include "tui/mapping.hpp"

namespace tui {
namespace component {

class View : public ModalView {
 public:
  View(ftxui::Component main_content, tui::Mode mode = tui::NORMAL);

  ftxui::Element OnRender() override;
  bool OnEvent(ftxui::Event event) override;
  void SetActiveChild(ftxui::ComponentBase *child) override;

  ftxui::Element RenderStatusBar(const KeyPairs &) const;

  void ShowModal(ftxui::Component component);
  void HideModal();

 protected:
  void BindKeymaps() override;

  ftxui::Component main_content_;

 private:
  bool show_modal_;
  bool show_prompt_;
  std::string prompt_value_;

  ftxui::Component modal_content_;
  std::shared_ptr<CommandInput> prompt_;

  std::stack<ftxui::ComponentBase *> focus_history_;

  void PopAndFocusTopComponent();
};

}  // namespace component
}  // namespace tui

#endif
