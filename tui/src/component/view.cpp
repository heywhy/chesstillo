#include <cstddef>
#include <utility>

#include <ftxui/component/component.hpp>
#include <ftxui/component/component_base.hpp>
#include <ftxui/component/event.hpp>
#include <ftxui/component/screen_interactive.hpp>

#include "tui/component/command_input.hpp"
#include "tui/component/modal_view.hpp"
#include "tui/component/view.hpp"
#include "tui/mapping.hpp"
#include "tui/utils.hpp"

namespace tui {
namespace component {

View::View(ftxui::Component main_content, tui::Mode mode)
    : ModalView(mode),
      main_content_(std::move(main_content)),
      show_modal_(false),
      show_prompt_(false),
      prompt_(tui::Make<CommandInput>(prompt_value_)) {
  Add(prompt_);
  Add(main_content_);

  main_content_->TakeFocus();
}

void View::BindKeymaps() {
  SetKeymap(tui::NORMAL | tui::VISUAL, "q", [] {
    auto screen = ftxui::ScreenInteractive::Active();

    if (screen) {
      screen->Post(screen->ExitLoopClosure());
    }
  });
}

ftxui::Element View::RenderStatusBar(const KeyPairs &pairs) const {
  const char *mode;
  ftxui::Elements menus;
  tui::Mode current_mode = Mode();

  switch (current_mode) {
    case tui::NORMAL:
      mode = " NORMAL ";
      break;

    case tui::VISUAL:
      mode = " VISUAL ";
      break;

    case tui::INTERACT:
      mode = " INTERACT ";
      break;
  }

  menus.push_back(ftxui::text(mode) | ftxui::bold);

  if (current_mode != tui::NORMAL) {
    menus.push_back(ftxui::separator());
    menus.push_back(ftxui::text(" <esc> ") | ftxui::bold);
    menus.push_back(ftxui::text("exit mode "));
  }

  for (const auto &pair : pairs) {
    menus.push_back(ftxui::separator());
    menus.push_back(ftxui::text(" "));
    menus.push_back(ftxui::text(pair.first.data()) | ftxui::bold);
    menus.push_back(ftxui::text(" "));
    menus.push_back(ftxui::text(pair.second.data()));
    menus.push_back(ftxui::text(" "));
  }

  if (current_mode & (tui::NORMAL | tui::VISUAL)) {
    menus.push_back(ftxui::separator());
    menus.push_back(ftxui::text(" q ") | ftxui::bold);
    menus.push_back(ftxui::text("quit "));

    if (!buffer_.empty()) {
      menus.push_back(ftxui::separator());
      menus.push_back(ftxui::text(" ") | ftxui::bold);
      menus.push_back(ftxui::text(buffer_) | ftxui::bold);
      menus.push_back(ftxui::text(" "));
    }
  }

  return ftxui::hbox(menus) | ftxui::borderLight;
}

bool View::OnEvent(ftxui::Event event) {
  if (show_prompt_ && event != ftxui::Event::Escape) {
    prompt_->OnEvent(event);

    return true;
  }

  if (show_prompt_ && event == ftxui::Event::Escape) {
    show_prompt_ = false;

    prompt_value_.clear();

    PopAndFocusTopComponent();

    return true;
  }

  if ((Mode() & (tui::NORMAL | tui::VISUAL)) && !show_prompt_ &&
      event.input() == ":") {
    show_prompt_ = true;

    prompt_->TakeFocus();

    return true;
  }

  if (show_modal_ && event != ftxui::Event::Escape &&
      !dynamic_cast<component::ModalView *>(modal_content_.get())) {
    modal_content_->OnEvent(event);

    return true;
  }

  if (show_modal_ && event == ftxui::Event::Escape) {
    HideModal();

    return true;
  }

  if (!ModalView::OnEvent(event)) {
    ActiveChild()->OnEvent(event);
  }

  return true;
}

void View::PopAndFocusTopComponent() {
  focus_history_.pop();

  if (!focus_history_.empty()) [[likely]] {
    focus_history_.top()->TakeFocus();
  }
}

void View::SetActiveChild(ftxui::ComponentBase *child) {
  ModalView::SetActiveChild(child);

  if (child->Active() &&
      (focus_history_.empty() || focus_history_.top() != child)) {
    focus_history_.push(child);
  }
}

void View::HideModal() {
  if (!show_modal_) {
    return;
  }

  show_modal_ = false;

  modal_content_->Detach();
  modal_content_ = nullptr;

  PopAndFocusTopComponent();
}

void View::ShowModal(ftxui::Component content) {
  if (show_modal_) {
    return;
  }

  show_modal_ = true;
  modal_content_ = content;

  Add(content);

  content->TakeFocus();
}

ftxui::Element View::OnRender() {
  ftxui::Elements elements;
  auto node = main_content_->Render();

  if (show_modal_) {
    elements.push_back(modal_content_->Render() | ftxui::clear_under |
                       ftxui::center);
  }

  if (show_prompt_) {
    elements.push_back(ftxui::vbox({
                           ftxui::text("\r\n"),
                           ftxui::text("\r\n"),
                           ftxui::text("\r\n"),
                           ftxui::text("\r\n"),
                           ChildAt(0)->Render() | ftxui::clear_under,
                       }) |
                       ftxui::size(ftxui::WIDTH, ftxui::EQUAL, 50) |
                       ftxui::hcenter);
  }

  if (!elements.empty()) {
    ftxui::Elements nodes = {node};

    ftxui::Merge(nodes, elements);

    node = ftxui::dbox(nodes);
  }

  return node;
}

}  // namespace component
}  // namespace tui
