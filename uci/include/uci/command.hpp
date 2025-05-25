#ifndef UCI_EXPRESSIONS_HPP
#define UCI_EXPRESSIONS_HPP

#include <cstdint>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "types.hpp"

namespace uci {
namespace command {
struct Debug;
struct SetOption;
struct Register;
struct Position;
struct Go;

struct Input;

struct ID;
struct BestMove;
struct CopyProtection;
struct Registration;
struct Info;
struct Option;
}  // namespace command

struct Visitor;

struct Command {
  virtual ~Command() {};
  virtual void Accept(Visitor &) = 0;
  virtual std::string ToString() const = 0;
};

struct Visitor {
  virtual void VisitInput(command::Input *) = 0;
  virtual void VisitDebug(command::Debug *) = 0;
  virtual void VisitPosition(command::Position *) = 0;
  virtual void VisitGo(command::Go *) = 0;
  virtual void VisitSetOption(command::SetOption *) = 0;
  virtual void VisitRegister(command::Register *) = 0;

  virtual void VisitID(command::ID *) = 0;
  virtual void VisitBestMove(command::BestMove *) = 0;
  virtual void VisitCopyProtection(command::CopyProtection *) = 0;
  virtual void VisitRegistration(command::Registration *) = 0;
  virtual void VisitInfo(command::Info *) = 0;
  virtual void VisitOption(command::Option *) = 0;
};

namespace command {
struct Input : public Command {
  const TokenType type;
  const std::string_view &input;

  static std::unordered_map<std::string_view, TokenType> Known;

  Input(const std::string_view &input) : Input(Known.at(input), input) {}

  Input(TokenType type, const std::string_view &input)
      : type(type), input(input) {}

  bool IsFeedback() {
    switch (type) {
      case UCI:
      case IS_READY:
      case UCI_NEW_GAME:
      case STOP:
      case PONDER_HIT:
      case QUIT:
        return false;

      case UCI_OK:
      case READY_OK:
        return true;

      default:
        return false;
    }
  }

  void Accept(Visitor &visitor) override { visitor.VisitInput(this); };

  std::string ToString() const override { return std::string(input); }
};

struct Debug : public Command {
  const bool value;

  Debug(bool value) : value(value) {}

  std::string ToString() const override;

  void Accept(Visitor &visitor) override { visitor.VisitDebug(this); };
};

struct Position : public Command {
  const std::string input;
  std::vector<std::string_view> moves;

  Position(const std::string_view &pos) : input(pos) {};

  std::string ToString() const override;

  void Accept(Visitor &visitor) override { visitor.VisitPosition(this); };
};

struct Go : public Command {
  std::vector<std::string_view> searchmoves;
  bool ponder = false;
  int wsec = -1;
  int bsec = -1;
  int winc = 0;
  int binc = 0;
  int movestogo = 0;
  int depth = 0;
  int nodes = 0;
  int mate = 0;
  int movetime = 0;
  bool infinite = false;

  std::string ToString() const override;

  void Accept(Visitor &visitor) override { visitor.VisitGo(this); };
};

struct SetOption : public Command {
  const std::string_view &id;
  Literal value;

  SetOption(const std::string_view &id) : id(id) {}
  SetOption(const std::string_view &id, auto value) : id(id), value(value) {}

  std::string ToString() const override;

  void Accept(Visitor &visitor) override { visitor.VisitSetOption(this); };
};

struct Register : public Command {
  bool later = false;
  std::string name;
  std::string_view code;

  std::string ToString() const override;

  void Accept(Visitor &visitor) override { visitor.VisitRegister(this); };
};

// Engine -> GUI

struct ID : Command {
  enum Type : uint8_t { NAME, AUTHOR };

  const Type type;
  std::string_view value;

  ID(Type type) : type(type) {}

  std::string ToString() const override;

  void Accept(Visitor &visitor) override { visitor.VisitID(this); }
};

struct BestMove : public Command {
  const std::string_view move;
  std::string_view ponder;

  BestMove(const std::string_view &move) : move(move) {}

  std::string ToString() const override;

  void Accept(Visitor &visitor) override { visitor.VisitBestMove(this); }
};

struct CopyProtection : public Command {
  const Status status;

  CopyProtection(Status status) : status(status) {}

  std::string ToString() const override;

  void Accept(Visitor &visitor) override { visitor.VisitCopyProtection(this); }
};

struct Registration : public Command {
  const Status status;

  Registration(Status status) : status(status) {}

  std::string ToString() const override;

  void Accept(Visitor &visitor) override { visitor.VisitRegistration(this); }
};

struct Info : public Command {
 public:
  struct Score {
    enum Type { CP, MATE };

    Type type;
    int value;
    bool lowerbound = false;
    bool upperbound = false;

    std::string ToString();
  };

  struct Currline {
    int cpunr;
    std::vector<std::string_view> moves;

    std::string ToString();
  };

  int depth{0};
  int seldepth{0};
  int multipv{0};
  Score *score;
  int nodes{0};
  int nps{0};
  int hashfull{0};
  int tbhits{0};
  int sbhits{0};
  int time{0};
  std::vector<std::string_view> pv;
  std::string_view currmove;
  int currmovenumber{0};
  int cpuload{0};
  std::vector<std::string_view> refutation;
  Currline *currline;

  std::string_view string;

  Info() : score(nullptr), currline(nullptr) {}

  ~Info() {
    delete score;
    delete currline;
  }

  std::string ToString() const override;

  void Accept(Visitor &visitor) override { visitor.VisitInfo(this); }
};

struct Option : public Command {
  OptionType type;
  std::string id;
  Literal def4ult;
  std::int64_t min = 0;
  std::int64_t max = 0;
  std::vector<std::string_view> vars;

  std::string ToString() const override;

  void Accept(Visitor &visitor) override { visitor.VisitOption(this); }
};

}  // namespace command
}  // namespace uci
#endif
