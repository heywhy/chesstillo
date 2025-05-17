#include <ftxui/component/component_base.hpp>
#include <ftxui/screen/box.hpp>
#include <tui/component/scroll_view.hpp>

namespace tui {
namespace component {

ScrollView::ScrollView(ftxui::Component child) : ScrollView(child, ALL) {}

ScrollView::ScrollView(ftxui::Component child, const Direction direction)
    : direction_(direction), scroll_width_(0), scroll_height_(0) {
  Add(child);
}

ftxui::Element ScrollView::OnRender() {
  ftxui::Element background = ftxui::ComponentBase::Render();
  background->ComputeRequirement();

  const int focusOffsetWidth = box_.x_max - box_.x_min - 1;
  const int focusOffsetHeight = box_.y_max - box_.y_min - 1;

  const ftxui::Requirement requirement = background->requirement();

  size_width_ = requirement.min_x - focusOffsetWidth;
  size_height_ = requirement.min_y - focusOffsetHeight;

  background |= ftxui::focusPosition(focusOffsetWidth / 2 + scroll_width_,
                                     focusOffsetHeight / 2 + scroll_height_);

  switch (direction_) {
  case ALL:
    background |= ftxui::vscroll_indicator;
    background |= ftxui::frame;
    background |= ftxui::flex;
    break;

  case WIDTH:
    background |= ftxui::vscroll_indicator;
    background |= ftxui::xframe;
    background |= ftxui::xflex;
    break;

  case HEIGHT:
    background |= ftxui::vscroll_indicator;
    background |= ftxui::yframe;
    background |= ftxui::yflex;
    break;
  }

  if (Focused()) {
    background |= ftxui::focus;
  }

  return std::move(background) | ftxui::reflect(box_);
}

bool ScrollView::OnEvent(ftxui::Event event) {
  if (ftxui::ComponentBase::OnEvent(event)) {
    return true;
  }

  ftxui::Mouse &mouse = event.mouse();

  if (event.is_mouse() && !box_.Contain(mouse.x, mouse.y)) {
    return false;
  }

  int scrolled_width = scroll_width_;
  int scrolled_height = scroll_height_;

  if (event == ftxui::Event::ArrowUp || event == ftxui::Event::Character('k') ||
      (event.is_mouse() && mouse.button == ftxui::Mouse::WheelUp)) {
    if (direction_ == WIDTH) {
      scroll_width_--;
    }

    scroll_height_--;
  }

  if (event == ftxui::Event::ArrowDown ||
      event == ftxui::Event::Character('j') ||
      (event.is_mouse() && mouse.button == ftxui::Mouse::WheelDown)) {
    if (direction_ == WIDTH) {
      scroll_width_++;
    }

    scroll_height_++;
  }

  if (event == ftxui::Event::ArrowLeft ||
      event == ftxui::Event::Character('h')) {
    scroll_width_--;
  }

  if (event == ftxui::Event::ArrowRight ||
      event == ftxui::Event::Character('l')) {
    scroll_width_++;
  }

  if (event == ftxui::Event::PageUp) {
    if (direction_ == WIDTH) {
      scroll_width_ -= box_.x_max - box_.x_min;
    }

    scroll_height_ -= box_.y_max - box_.y_min;
  }

  if (event == ftxui::Event::PageDown) {
    if (direction_ == WIDTH) {
      scroll_width_ += box_.x_max - box_.x_min;
    }

    scroll_height_ += box_.y_max - box_.y_min;
  }

  if (event == ftxui::Event::Home) {
    scroll_width_ = 0;
    scroll_height_ = 0;
  }

  if (event == ftxui::Event::End) {
    scroll_width_ = size_width_;
    scroll_height_ = size_height_;
  }

  scroll_width_ = std::max(0, std::min(size_width_ - 1, scroll_width_));
  scroll_height_ = std::max(0, std::min(size_height_ - 1, scroll_height_));

  return scrolled_width != scroll_width_ || scrolled_height != scroll_height_;
}

} // namespace component
} // namespace tui
