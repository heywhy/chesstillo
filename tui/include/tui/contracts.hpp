#ifndef TUI_CONTRACTS_HPP
#define TUI_CONTRACTS_HPP

namespace tui {

class HasKeymaps {
 public:
  static void Bind(HasKeymaps *ptr) { ptr->BindKeymaps(); }

 private:
  virtual void BindKeymaps() = 0;
};

}  // namespace tui

#endif
