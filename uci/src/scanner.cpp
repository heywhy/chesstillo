#include <cctype>
#include <stdexcept>
#include <string_view>
#include <unordered_map>

#include <uci/scanner.hpp>
#include <uci/types.hpp>

using namespace uci;

static const std::unordered_map<std::string_view, uci::TokenType> kKeywords{
    {"uci", UCI},
    {"debug", DEBUG},
    {"isready", IS_READY},
    {"setoption", SET_OPTION},
    {"register", REGISTER},
    {"ucinewgame", UCI_NEW_GAME},
    {"position", POSITION},
    {"go", GO},
    {"stop", STOP},
    {"ponderhit", PONDER_HIT},
    {"quit", QUIT},

    {"id", ID},
    {"uciok", UCI_OK},
    {"readyok", READY_OK},
    {"bestmove", BEST_MOVE},
    {"copyprotection", COPY_PROTECTION},
    {"registration", REGISTRATION},
    {"info", INFO},
    {"option", OPTION},
    {"true", BOOLEAN},
    {"false", BOOLEAN},
};

Scanner::Scanner(const std::string_view &input)
    : input_(input), line_(1), start_(0), current_(0) {}

Tokens &Scanner::Scan() {
  while (!IsAtEnd()) {
    start_ = current_;
    ScanToken();
  }

  return tokens_;
}

void Scanner::ScanToken() {
  char c = Advance();

  switch (c) {
    case ' ':
    case '\r':
    case '\t':
      break;

    case '\n':
      line_++;
      break;

    case '-':
      if (std::isdigit(Peek())) {
        Digit();
      } else {
        Word();
      }
      break;

    default:
      if (std::isdigit(c)) {
        Digit();
      } else {
        Word();
      }
      break;
  }
}

void Scanner::Word() {
  while (!IsAtEnd() && Peek() != ' ' && Peek() != '\n' && Peek() != '\t') {
    Advance();
  }

  auto value = input_.substr(start_, current_ - start_);
  TokenType type = kKeywords.contains(value) ? kKeywords.at(value) : WORD;

  if (type == BOOLEAN) {
    AddToken(type, value == "true");
  } else {
    AddToken(type, value);
  }
}

void Scanner::Digit() {
  while (!IsAtEnd()) {
    if (std::isdigit(Peek())) {
      Advance();
      continue;
    }

    // INFO: convert to word if character is not a terminator.
    if (Peek() != ' ' && Peek() != '\n' && Peek() != '\t') {
      return Word();
    }

    break;
  }

  try {
    auto word = input_.substr(start_, current_ - start_);
    int value = std::stoi(std::string(word));

    AddToken(NUMBER, value);
  } catch (std::out_of_range &) {
    Word();
  }
}

char Scanner::Peek() {
  if (IsAtEnd()) {
    return '\0';
  }

  return input_.at(current_);
}

char Scanner::Advance() { return input_.at(current_++); }

void Scanner::AddToken(TokenType type) { AddToken(type, -1); }

void Scanner::AddToken(TokenType type, auto value) {
  auto text = input_.substr(start_, current_ - start_);

  tokens_.emplace_back(type, text, value, start_ + 1, line_);
}

bool Scanner::IsAtEnd() { return current_ >= input_.size(); }
