#include <cstddef>
#include <mutex>
#include <thread>
#include <utility>

#include <ftxui/component/component_base.hpp>
#include <ftxui/component/event.hpp>
#include <ftxui/component/screen_interactive.hpp>

#include <tui/component/modal_view.hpp>
#include <tui/contracts.hpp>
#include <tui/mapping.hpp>
#include <tui/utility.hpp>

using namespace std::chrono_literals;

namespace tui {
namespace component {

ModalView::ModalView(tui::Mode mode)
    : mode_(mode),
      old_mode_(mode),
      focused_(0),
      child_modal_view_(nullptr),
      loop_ready_(false),
      wait_to_handle_mapping_(true),
      thread_(&ModalView::Loop, this) {
  std::unique_lock lock(mutex_);

  buffer_.reserve(MAX_SEQUENCE_LEN);

  cv_.wait(lock, [this] { return loop_ready_; });
}

ModalView::~ModalView() {
  std::unique_lock lock(mutex_);

  wait_to_handle_mapping_ = false;

  lock.unlock();
  cv_.notify_all();

  thread_.join();

  UnfocusView();
}

ModalView *ModalView::Topmost() {
  ftxui::ComponentBase *child = this;
  ModalView *topmost_modal_view = nullptr;

  while (ftxui::ComponentBase *parent = child->Parent()) {
    if (dynamic_cast<ModalView *>(parent)) {
      topmost_modal_view = dynamic_cast<ModalView *>(parent);
    }

    child = parent;
  }

  return topmost_modal_view;
}

void ModalView::FocusView() {
  ModalView *topmost_modal_view = Topmost();

  if (topmost_modal_view) {
    topmost_modal_view->child_modal_view_ = this;
  }
}

void ModalView::UnfocusView() {
  ModalView *topmost_modal_view = Topmost();

  if (topmost_modal_view && topmost_modal_view->child_modal_view_ == this) {
    topmost_modal_view->child_modal_view_ = nullptr;
  }
}

void ModalView::SetActiveChild(ftxui::ComponentBase *child) {
  for (std::size_t i = 0; i < children_.size(); i++) {
    if (children_[i].get() == child) {
      focused_ = i;
      auto modal_view = dynamic_cast<ModalView *>(child);

      if (modal_view) {
        modal_view->FocusView();
      }

      break;
    }
  }
}

ftxui::Component ModalView::ActiveChild() {
  if (focused_ >= children_.size()) {
    return nullptr;
  }

  return children_[focused_];
}

bool ModalView::OnEvent(ftxui::Event event) {
  if (child_modal_view_ && child_modal_view_ == ActiveChild().get() &&
      child_modal_view_->OnEvent(event)) {
    return true;
  }

  bool handled;

  switch (mode_) {
    case NORMAL:
      handled = HandleNormalEvent(event);
      break;

    case VISUAL:
      handled = HandleVisualEvent(event);
      break;

    case INTERACT:
      handled = HandleInteractEvent(event);
      break;
  }

  return handled;
}

bool ModalView::HandleNormalEvent(const ftxui::Event &event) {
  if (event == ftxui::Event::i) {
    SwitchTo(tui::INTERACT);

    return true;
  }

  if (event == ftxui::Event::v) {
    SwitchTo(tui::VISUAL);

    return true;
  }

  MaybeWriteToBuffer(event);

  if (MaybeApplyKeymap(event)) {
    return true;
  }

  if (event == ftxui::Event::Escape) {
    buffer_.clear();
  }

  return true;
}

bool ModalView::HandleVisualEvent(const ftxui::Event &event) {
  if (event == ftxui::Event::Escape) {
    std::swap(old_mode_, mode_);

    return true;
  }

  MaybeWriteToBuffer(event);

  if (MaybeApplyKeymap(event)) {
    return true;
  }

  return !event.is_mouse();
}

bool ModalView::HandleInteractEvent(const ftxui::Event &event) {
  if (event == ftxui::Event::Escape) {
    std::swap(old_mode_, mode_);

    return true;
  }

  return event.is_mouse();
}

bool ModalView::MaybeApplyKeymap(const ftxui::Event &event) {
  if (!buffer_.empty() && Handle(mode_, buffer_)) {
    !event.screen_ ? buffer_.clear()
                   : event.screen_->Post([this] { buffer_.clear(); });

    return true;
  }

  return false;
}

void ModalView::MaybeWriteToBuffer(const ftxui::Event &event) {
  if (!event.is_mouse() && event != ftxui::Event::Custom) {
    buffer_.append(event.input());
    cv_.notify_all();
  }
}

void ModalView::Loop() {
  {
    std::lock_guard lock(mutex_);

    loop_ready_ = true;

    cv_.notify_all();
  }

  std::unique_lock lock(mutex_);
  std::string last_buffer(buffer_);

  while (loop_ready_) {
    cv_.wait(lock, [this, &last_buffer] {
      return (!buffer_.empty() && buffer_ != last_buffer) ||
             !wait_to_handle_mapping_;
    });

    if (!wait_to_handle_mapping_) {
      break;
    }

    // TODO: wait for 30ms before trying to handle the keymap.
    std::this_thread::sleep_for(30ms);

    last_buffer = buffer_;

    const auto screen = ftxui::ScreenInteractive::Active();

    if (screen) {
      screen->PostEvent(ftxui::Event::Custom);
    }
  }
}

}  // namespace component
}  // namespace tui
