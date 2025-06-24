#ifndef UCI_TYPES_HPP
#define UCI_TYPES_HPP

#include <cstdint>
#include <string_view>
#include <unordered_map>
#include <variant>
#include <vector>

namespace uci {

enum class TokenType {
  // GUI to Engine
  UCI,
  DEBUG,
  IS_READY,
  SET_OPTION,
  REGISTER,
  UCI_NEW_GAME,
  POSITION,
  GO,
  STOP,
  PONDER_HIT,
  QUIT,

  // Engine to GUI
  ID,
  UCI_OK,
  READY_OK,
  BEST_MOVE,
  COPY_PROTECTION,
  REGISTRATION,
  INFO,
  OPTION,

  // others
  WORD,
  NUMBER,
  BOOLEAN
};

enum class Status { CHECKING, OK, ERROR };
enum class OptionType { CHECK, SPIN, COMBO, BUTTON, STRING };

extern std::unordered_map<std::string_view, Status> kStatus;

using Literal = std::variant<std::string_view, std::int64_t, bool>;

struct Token {
  TokenType type;
  const std::string_view lexeme;
  Literal literal;
  int col;
  int line;
};

using Tokens = std::vector<Token>;

}  // namespace uci

#endif
