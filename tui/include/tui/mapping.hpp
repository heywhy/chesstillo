#ifndef TUI_MAPPING_HPP
#define TUI_MAPPING_HPP

#include <array>
#include <cstddef>
#include <cstdint>
#include <forward_list>
#include <functional>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

#define MAX_MAPHASH 256
#define MAX_SEQUENCE_LEN 50
#define MAP_HASH(mode, c) \
  (mode & (tui::NORMAL | tui::VISUAL)) ? c : ((c) ^ 0x80)

namespace tui {

using Mode = std::uint8_t;

constexpr const Mode NORMAL = 1 << 0;
constexpr const Mode VISUAL = 1 << 1;
constexpr const Mode INTERACT = 1 << 2;

namespace mapping {

using RHS = std::variant<std::monostate, std::function<void()>, std::string>;

struct BindOption {
  std::string_view desc;
  bool wait = false;
  bool unique = false;
  bool silent = false;

  // INFO: use?
  bool expr = false;
  bool replace_keycodes = false;
};

struct BindArg : public BindOption {
  std::string lhs;
  std::string alt_lhs;

  RHS alt_rhs;
  RHS rhs;
  bool rhs_is_noop = false;

  BindArg() = default;

  BindArg(const BindOption &option) {
    desc = option.desc;
    wait = option.wait;
    unique = option.unique;
    silent = option.silent;

    expr = option.expr;
    replace_keycodes = option.replace_keycodes;
  }
};

struct Entry {
  Mode mode;
  std::string_view desc;
  bool wait;
  bool silent;

  std::string lhs;
  std::string orig_lhs;
  RHS rhs;
  bool rhs_is_noop;
};

std::string ReplaceTermcodes(const std::string_view &lhs, bool simplify = true);
bool SetSides(BindArg &arg, const std::string_view &lhs, const RHS &rhs);
}  // namespace mapping

class Mapping {
 public:
  void SetKeymap(Mode mode, const std::string_view lhs, mapping::RHS rhs,
                 mapping::BindOption option = {});

  void DelKeymap(Mode mode, const std::string_view lhs);

  std::vector<const mapping::Entry *> GetKeymaps(Mode mode);

  bool Handle(Mode mode, std::string_view buffer);

 private:
  std::array<std::forward_list<mapping::Entry>, MAX_MAPHASH> map_table_;
};

}  // namespace tui

#endif
