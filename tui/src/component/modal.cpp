#include <functional>

#include <ftxui/component/component_base.hpp>
#include <ftxui/dom/elements.hpp>

#include <tui/component/modal.hpp>

namespace tui {
namespace component {

Modal::Modal(ftxui::Component main, ftxui::Component content)
    : show_(false), main_(std::move(main)), content_(std::move(content)) {
  auto hide_fn = std::bind(&Modal::Hide, this);

  SetKeymap(tui::NORMAL, "q", hide_fn);
  SetKeymap(tui::NORMAL, "<esc>", hide_fn);

  Add(ftxui::Modal(main_, content_, &show_));
}

ftxui::Element Modal::OnRender() {
  auto document = main_->Render();

  if (show_) {
    // TODO: support content alignment and not clear under if not center
    document = ftxui::dbox({
        document,
        content_->Render() | ftxui::clear_under | ftxui::center,
    });
  }

  return document;
}

}  // namespace component
}  // namespace tui
