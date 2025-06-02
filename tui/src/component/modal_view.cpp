#include <mutex>
#include <thread>

#include <ftxui/component/component_base.hpp>
#include <ftxui/component/event.hpp>
#include <ftxui/component/screen_interactive.hpp>

#include <tui/component/modal_view.hpp>
#include <tui/mapping.hpp>
#include <tui/utility.hpp>

using namespace std::chrono_literals;

namespace tui {
namespace component {

ModalView::ModalView() : ModalView(NORMAL) {}

ModalView::ModalView(tui::Mode mode)
    : mode_(mode),
      loop_ready_(false),
      wait_to_handle_mapping_(true),
      thread_(&ModalView::Loop, this) {
  std::unique_lock lock(mutex_);

  buffer_.reserve(MAX_SEQUENCE_LEN);

  cv_.wait(lock, [this] { return loop_ready_; });

  SetKeymap(tui::NORMAL | tui::VISUAL, "q", [] {
    if (ActiveScreen()) {
      ActiveScreen()->Post(ActiveScreen()->ExitLoopClosure());
    }
  });
}

ModalView::~ModalView() {
  std::unique_lock lock(mutex_);

  wait_to_handle_mapping_ = false;

  lock.unlock();
  cv_.notify_all();

  thread_.join();
}

bool ModalView::OnEvent(ftxui::Event event) {
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

  return handled ? true : ftxui::ComponentBase::OnEvent(event);
}

ftxui::Element ModalView::RenderKeymaps(const KeyPairs &pairs) const {
  const char *mode;
  ftxui::Elements menus;

  switch (mode_) {
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

  if (mode_ != tui::NORMAL) {
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

  if (mode_ & (tui::NORMAL | tui::VISUAL)) {
    menus.push_back(ftxui::separator());
    menus.push_back(ftxui::text(" q ") | ftxui::bold);
    menus.push_back(ftxui::text("quit "));
  }

  return ftxui::hbox({ftxui::hbox(menus) | ftxui::borderLight});
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

  if (MaybeApplyKeymap(event.screen_)) {
    return true;
  }

  if (event == ftxui::Event::Escape) {
    buffer_.clear();

    return true;
  }

  return true;
}

bool ModalView::HandleVisualEvent(const ftxui::Event &event) {
  if (event == ftxui::Event::Escape) {
    SwitchTo(tui::NORMAL);

    return true;
  }

  MaybeWriteToBuffer(event);

  if (MaybeApplyKeymap(event.screen_)) {
    return true;
  }

  return !event.is_mouse();
}

bool ModalView::HandleInteractEvent(const ftxui::Event &event) {
  if (event == ftxui::Event::Escape) {
    SwitchTo(tui::NORMAL);

    return true;
  }

  return event.is_mouse();
}

bool ModalView::MaybeApplyKeymap(ftxui::ScreenInteractive *screen) {
  if (!buffer_.empty() && Handle(mode_, buffer_)) {
    !screen ? buffer_.clear() : screen->Post([this] { buffer_.clear(); });

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

    const auto screen = ActiveScreen();

    if (screen) {
      screen->PostEvent(ftxui::Event::Custom);
    }
  }
}

}  // namespace component
}  // namespace tui
