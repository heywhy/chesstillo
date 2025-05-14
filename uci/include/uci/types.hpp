#ifndef UCI_TYPES_HPP
#define UCI_TYPES_HPP

#include <cstdint>
#include <string_view>
#include <unordered_map>
#include <variant>
#include <vector>

namespace uci {

enum TokenType : uint8_t {
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
  NUMBER
};

enum Status : uint8_t { CHECKING, OK, ERROR };
enum OptionType : uint8_t { CHECK, SPIN, COMBO, BUTTON, STRING };

extern std::unordered_map<std::string_view, Status> kStatus;

struct Token {
  TokenType type;
  const std::string_view lexeme;
  std::variant<std::string_view, int> literal;
  int col;
  int line;
};

typedef std::vector<Token> Tokens;

} // namespace uci

#endif
