#ifndef TUI_COMPONENT_MODAL_VIEW_HPP
#define TUI_COMPONENT_MODAL_VIEW_HPP

#include <condition_variable>
#include <cstddef>
#include <mutex>
#include <string>
#include <string_view>
#include <thread>
#include <utility>
#include <vector>

#include <ftxui/component/component.hpp>
#include <ftxui/component/event.hpp>
#include <ftxui/component/screen_interactive.hpp>

#include <tui/contracts.hpp>
#include <tui/mapping.hpp>

#define MAX_MAPHASH 256
#define MAX_SEQUENCE_LEN 50

namespace tui {
namespace component {

// TODO:
// * support editable attr and don't bind "i" to enter the interact mode when
// it's false.
// * remove default keymaps
class ModalView : public Mapping,
                  public HasKeymaps,
                  public ftxui::ComponentBase {
 public:
  using KeyPair = std::pair<std::string_view, std::string_view>;
  using KeyPairs = std::vector<KeyPair>;

  ModalView();
  ModalView(tui::Mode mode);
  ~ModalView();

  bool OnEvent(ftxui::Event) override;
  bool Focusable() const override { return true; };
  ftxui::Element RenderStatusBar(const KeyPairs &) const;

  void FocusView();
  void UnfocusView();

  inline tui::Mode Mode() const { return mode_; }

 protected:
  ModalView *Topmost();

 private:
  tui::Mode mode_;
  tui::Mode old_mode_;
  std::string buffer_;

  ModalView *child_modal_view_;

  bool loop_ready_;
  bool wait_to_handle_mapping_;

  std::mutex mutex_;
  std::thread thread_;
  std::condition_variable cv_;

  void BindKeymaps() override;

  void Loop();
  bool HandleNormalEvent(const ftxui::Event &event);
  bool HandleVisualEvent(const ftxui::Event &event);
  bool HandleInteractEvent(const ftxui::Event &event);

  void MaybeWriteToBuffer(const ftxui::Event &event);
  bool MaybeApplyKeymap(ftxui::ScreenInteractive *screen);

  inline void SwitchTo(tui::Mode mode) {
    old_mode_ = mode_;
    mode_ = mode;
    buffer_.clear();
  }
};

}  // namespace component
}  // namespace tui

#endif
