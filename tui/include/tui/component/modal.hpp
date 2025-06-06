#ifndef TUI_COMPONENT_MODAL_HPP
#define TUI_COMPONENT_MODAL_HPP

#include <ftxui/component/component.hpp>
#include <ftxui/dom/elements.hpp>

#include <tui/color.hpp>
#include <tui/component/modal_view.hpp>

namespace tui {
namespace component {

class Modal : public ModalView {
 public:
  Modal(ftxui::Component main, ftxui::Component content);

  ftxui::Element OnRender() override;

  inline bool Visible() { return show_; }

  inline void Hide() {
    show_ = false;

    UnfocusView();
  }

  inline void Show() {
    show_ = true;

    FocusView();
  }

  inline void Toggle() { show_ ? Hide() : Show(); }

 private:
  bool show_;
  ftxui::Component main_;
  ftxui::Component content_;

  void BindKeymaps() override;
};

}  // namespace component
}  // namespace tui
#endif
