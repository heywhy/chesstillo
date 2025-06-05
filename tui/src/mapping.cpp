#include <algorithm>
#include <cctype>
#include <cstddef>
#include <cstdio>
#include <format>
#include <functional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>

#include <ftxui/component/event.hpp>

#include <tui/constants.hpp>
#include <tui/mapping.hpp>

namespace tui {

void Mapping::SetKeymap(Mode mode, const std::string_view lhs, mapping::RHS rhs,
                        mapping::BindOption option) {
  mapping::BindArg arg = option;

  if (!SetSides(arg, lhs, rhs)) {
    throw std::runtime_error("LHS exceeds maximum length.");
  }

  if (arg.lhs.size() > MAX_SEQUENCE_LEN ||
      arg.alt_lhs.size() > MAX_SEQUENCE_LEN) {
    throw std::runtime_error("LHS exceeds maximum length.");
  }

  if (arg.lhs.empty()) {
    throw std::runtime_error("Invalid (empty) LHS.");
  }

  if (std::holds_alternative<std::string>(arg.rhs) && !arg.rhs_is_noop) {
    std::string &v = std::get<std::string>(arg.rhs);

    if (v.empty()) {
      arg.rhs_is_noop = true;
    } else {
      std::string message =
          std::format("Unexpected RHS value for {}.", arg.alt_lhs);

      throw std::runtime_error(message);
    }
  }

  int hash = MAP_HASH(mode, static_cast<std::uint8_t>(arg.lhs[0]));
  auto &block = map_table_[hash];

  mapping::Entry new_entry;

  new_entry.mode = mode;
  new_entry.desc = arg.desc;
  new_entry.wait = arg.wait;
  new_entry.silent = arg.silent;

  new_entry.lhs = arg.lhs;
  new_entry.orig_lhs = arg.alt_lhs;
  new_entry.rhs = arg.rhs;
  new_entry.rhs_is_noop = arg.rhs_is_noop;

  for (auto it = block.begin(); it != block.end(); it++) {
    if (!(it->mode & mode)) {
      continue;
    }

    if ((it->lhs == new_entry.lhs && it->mode & mode) || !it->mode) {
      it->mode &= ~mode;

      if (!it->mode) {
        new_entry.mode = it->mode;
        *it = new_entry;

        return;
      }
    }
  }

  block.push_front(std::move(new_entry));
}

void Mapping::DelKeymap(Mode mode, const std::string_view lhs) {
  mapping::BindArg arg;

  if (!SetSides(arg, lhs, "")) {
    throw std::runtime_error("LHS exceeds maximum length.");
  }

  int hash = MAP_HASH(mode, static_cast<std::uint8_t>(arg.lhs[0]));
  auto &block = map_table_[hash];

  for (auto &entry : block) {
    if (entry.lhs == arg.lhs && entry.mode & mode) {
      entry.mode &= ~mode;
      return;
    }
  }
}

bool Mapping::Handle(Mode mode, std::string_view buffer) {
  int hash = MAP_HASH(mode, buffer[0]);
  auto block = map_table_[hash];

  for (auto &entry : block) {
    if (!(entry.mode & mode)) {
      continue;
    }

    if (entry.lhs == buffer && !entry.rhs_is_noop) {
      auto cb = std::get<std::function<void()>>(entry.rhs);

      // TODO: use bind options?
      cb();

      return true;
    }
  }

  return false;
}

std::vector<const mapping::Entry *> Mapping::GetKeymaps(Mode mode) {
  std::vector<const mapping::Entry *> result;

  for (auto &block : map_table_) {
    for (const auto &entry : block) {
      if (!(entry.mode & mode)) {
        continue;
      }

      result.push_back(&entry);
    }
  }

  return result;
}

namespace mapping {
std::size_t TransSpecial(std::string &result, const char *src, bool simplify);
void SetRHS(BindArg &arg, const RHS &rhs);

bool SetSides(BindArg &arg, const std::string_view &lhs, const RHS &rhs) {
  std::string replaced = ReplaceTermcodes(lhs);

  if (replaced.empty()) {
    return false;
  }

  arg.lhs = replaced;
  arg.alt_lhs = ReplaceTermcodes(lhs, false);

  SetRHS(arg, rhs);

  return true;
}

void SetRHS(BindArg &arg, const RHS &rhs) {
  std::visit(
      [&](auto &value) {
        using T = std::decay_t<decltype(value)>;

        if constexpr (std::is_same_v<T, std::string>) {
          if (value == "<NOP>" || value == "<nop>") {
            arg.rhs = "";
            arg.rhs_is_noop = true;
          } else {
            arg.rhs = value;
          }
        }

        if constexpr (std::is_invocable_v<T>) {
          arg.rhs = value;
          arg.rhs_is_noop = false;
        }
      },
      rhs);
}

std::string ReplaceTermcodes(const std::string_view &str, bool simplify) {
  std::string result;

  for (auto it = str.begin(); it != str.end();) {
    if (*it == '<') {
      it += TransSpecial(result, it, simplify);
      continue;
    }

    result += *it;

    it++;
  }

  return result;
}

std::size_t TransSpecial(std::string &result, const char *src, bool simplify) {
  if (*src != '<') {
    std::string message =
        std::format("Can't translate non-special key {}.", src);

    throw std::runtime_error(message);
  }

  int len = 1;
  std::string_view view(src);

  static std::unordered_map<std::string_view, std::string> M = {
      {"<UP>", ftxui::Event::ArrowUp.input()},
      {"<DOWN>", ftxui::Event::ArrowDown.input()},
      {"<LEFT>", ftxui::Event::ArrowLeft.input()},
      {"<RIGHT>", ftxui::Event::ArrowRight.input()},
      {"<SPACE>", kSpaceChar},
      {"<ESC>", ftxui::Event::Escape.input()},
      {"<C-A>", ftxui::Event::CtrlA.input()},
      {"<C-B>", ftxui::Event::CtrlB.input()},
      {"<C-C>", ftxui::Event::CtrlC.input()},
      {"<C-D>", ftxui::Event::CtrlD.input()},
      {"<C-E>", ftxui::Event::CtrlE.input()},
      {"<C-F>", ftxui::Event::CtrlF.input()},
      {"<C-G>", ftxui::Event::CtrlG.input()},
      {"<C-H>", ftxui::Event::CtrlH.input()},
      {"<C-I>", ftxui::Event::CtrlI.input()},
      {"<C-J>", ftxui::Event::CtrlJ.input()},
      {"<C-K>", ftxui::Event::CtrlK.input()},
      {"<C-L>", ftxui::Event::CtrlL.input()},
      {"<C-M>", ftxui::Event::CtrlM.input()},
      {"<C-N>", ftxui::Event::CtrlN.input()},
      {"<C-O>", ftxui::Event::CtrlO.input()},
      {"<C-P>", ftxui::Event::CtrlP.input()},
      {"<C-Q>", ftxui::Event::CtrlQ.input()},
      {"<C-R>", ftxui::Event::CtrlR.input()},
      {"<C-S>", ftxui::Event::CtrlS.input()},
      {"<C-T>", ftxui::Event::CtrlT.input()},
      {"<C-U>", ftxui::Event::CtrlU.input()},
      {"<C-V>", ftxui::Event::CtrlV.input()},
      {"<C-W>", ftxui::Event::CtrlW.input()},
      {"<C-X>", ftxui::Event::CtrlX.input()},
      {"<C-Y>", ftxui::Event::CtrlY.input()},
      {"<C-Z>", ftxui::Event::CtrlZ.input()},
      {"<A-A>", ftxui::Event::AltA.input()},
      {"<A-B>", ftxui::Event::AltB.input()},
      {"<A-C>", ftxui::Event::AltC.input()},
      {"<A-D>", ftxui::Event::AltD.input()},
      {"<A-E>", ftxui::Event::AltE.input()},
      {"<A-F>", ftxui::Event::AltF.input()},
      {"<A-G>", ftxui::Event::AltG.input()},
      {"<A-H>", ftxui::Event::AltH.input()},
      {"<A-I>", ftxui::Event::AltI.input()},
      {"<A-J>", ftxui::Event::AltJ.input()},
      {"<A-K>", ftxui::Event::AltK.input()},
      {"<A-L>", ftxui::Event::AltL.input()},
      {"<A-M>", ftxui::Event::AltM.input()},
      {"<A-N>", ftxui::Event::AltN.input()},
      {"<A-O>", ftxui::Event::AltO.input()},
      {"<A-P>", ftxui::Event::AltP.input()},
      {"<A-Q>", ftxui::Event::AltQ.input()},
      {"<A-R>", ftxui::Event::AltR.input()},
      {"<A-S>", ftxui::Event::AltS.input()},
      {"<A-T>", ftxui::Event::AltT.input()},
      {"<A-U>", ftxui::Event::AltU.input()},
      {"<A-V>", ftxui::Event::AltV.input()},
      {"<A-W>", ftxui::Event::AltW.input()},
      {"<A-X>", ftxui::Event::AltX.input()},
      {"<A-Y>", ftxui::Event::AltY.input()},
      {"<A-Z>", ftxui::Event::AltZ.input()},
      {"<C-A-A>", ftxui::Event::CtrlAltA.input()},
      {"<C-A-B>", ftxui::Event::CtrlAltB.input()},
      {"<C-A-C>", ftxui::Event::CtrlAltC.input()},
      {"<C-A-D>", ftxui::Event::CtrlAltD.input()},
      {"<C-A-E>", ftxui::Event::CtrlAltE.input()},
      {"<C-A-F>", ftxui::Event::CtrlAltF.input()},
      {"<C-A-G>", ftxui::Event::CtrlAltG.input()},
      {"<C-A-H>", ftxui::Event::CtrlAltH.input()},
      {"<C-A-I>", ftxui::Event::CtrlAltI.input()},
      {"<C-A-J>", ftxui::Event::CtrlAltJ.input()},
      {"<C-A-K>", ftxui::Event::CtrlAltK.input()},
      {"<C-A-L>", ftxui::Event::CtrlAltL.input()},
      {"<C-A-M>", ftxui::Event::CtrlAltM.input()},
      {"<C-A-N>", ftxui::Event::CtrlAltN.input()},
      {"<C-A-O>", ftxui::Event::CtrlAltO.input()},
      {"<C-A-P>", ftxui::Event::CtrlAltP.input()},
      {"<C-A-Q>", ftxui::Event::CtrlAltQ.input()},
      {"<C-A-R>", ftxui::Event::CtrlAltR.input()},
      {"<C-A-S>", ftxui::Event::CtrlAltS.input()},
      {"<C-A-T>", ftxui::Event::CtrlAltT.input()},
      {"<C-A-U>", ftxui::Event::CtrlAltU.input()},
      {"<C-A-V>", ftxui::Event::CtrlAltV.input()},
      {"<C-A-W>", ftxui::Event::CtrlAltW.input()},
      {"<C-A-X>", ftxui::Event::CtrlAltX.input()},
      {"<C-A-Y>", ftxui::Event::CtrlAltY.input()},
      {"<C-A-Z>", ftxui::Event::CtrlAltZ.input()},
  };

  while (*src) {
    if (*src == '>') {
      break;
    }

    len++;
    src++;
  }

  std::string_view substr = view.substr(0, len);
  std::string combo(substr);

  std::transform(combo.begin(), combo.end(), combo.begin(),
                 [](char c) { return std::toupper(c); });

  if (M.contains(combo)) {
    result.append(simplify ? M[combo] : combo);
  } else if ((combo.starts_with("<C-S-") || combo.starts_with("<C-") ||
              combo.starts_with("<S-")) &&
             combo.ends_with(">")) {
    std::string_view temp = substr;
    std::size_t s = combo.starts_with("<C-S-") ? 5 : 3;

    temp.remove_prefix(s);
    temp.remove_suffix(1);

    if (temp.size() == 1 && combo.starts_with("<C-")) {
      char s[6];

      std::snprintf(s, 6, "<C-%c>", std::toupper(temp[0]));

      TransSpecial(result, s, simplify);
    } else if (temp.size() == 1) {
      result += std::toupper(temp[0]);
    } else {
      result.append(substr);
    }

  } else {
    result.append(substr);
  }

  return len;
}

}  // namespace mapping
}  // namespace tui
