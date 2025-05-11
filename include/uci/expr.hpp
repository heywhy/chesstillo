#ifndef UCI_EXPRESSIONS_HPP
#define UCI_EXPRESSIONS_HPP

#include <cstdint>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

#include "types.hpp"

namespace uci {
namespace expr {
struct Debug;
struct SetOption;
struct Register;
struct Position;
struct Go;

struct Command;

struct ID;
struct BestMove;
struct CopyProtection;
struct Registration;
struct Info;
struct Option;
} // namespace expr

struct Visitor;

struct Expr {
  virtual ~Expr() {};
  virtual void Accept(Visitor &) = 0;
};

struct Visitor {
  virtual void VisitCommand(expr::Command *) = 0;
  virtual void VisitPosition(expr::Position *) = 0;
  virtual void VisitGo(expr::Go *) = 0;
  virtual void VisitSetOption(expr::SetOption *) = 0;
  virtual void VisitRegister(expr::Register *) = 0;

  virtual void VisitID(expr::ID *) = 0;
  virtual void VisitBestMove(expr::BestMove *) = 0;
  virtual void VisitCopyProtection(expr::CopyProtection *) = 0;
  virtual void VisitRegistration(expr::Registration *) = 0;
  virtual void VisitInfo(expr::Info *) = 0;
  virtual void VisitOption(expr::Option *) = 0;
};

namespace expr {
struct Command : public Expr {
  const std::string_view &input;

  Command(const std::string_view &command) : input(command) {}

  void Accept(Visitor &visitor) override { visitor.VisitCommand(this); };
};

struct Position : public Expr {
  const std::string input;
  std::vector<std::string_view> moves;

  Position(const std::string_view &pos) : input(pos) {};

  void Accept(Visitor &visitor) override { visitor.VisitPosition(this); };
};

struct Go : public Expr {
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
  bool infinite = false;

  void Accept(Visitor &visitor) override { visitor.VisitGo(this); };
};

struct SetOption : public Expr {
  const std::string_view &id;
  std::string value;

  SetOption(const std::string_view &id) : id(id) {}

  void Accept(Visitor &visitor) override { visitor.VisitSetOption(this); };
};

struct Register : public Expr {
  bool later = false;
  std::string name;
  std::string_view code;

  void Accept(Visitor &visitor) override { visitor.VisitRegister(this); };
};

// Engine -> GUI

struct ID : Expr {
  enum Type : uint8_t { NAME, AUTHOR };

  const Type type;
  std::string_view value;

  ID(Type type) : type(type) {}

  void Accept(Visitor &visitor) { visitor.VisitID(this); }
};

struct BestMove : public Expr {
  const std::string_view move;
  std::string_view ponder;

  BestMove(const std::string_view &move) : move(move) {}

  void Accept(Visitor &visitor) override { visitor.VisitBestMove(this); }
};

struct CopyProtection : public Expr {
  const Status status;

  CopyProtection(Status status) : status(status) {}

  void Accept(Visitor &visitor) override { visitor.VisitCopyProtection(this); }
};

struct Registration : public Expr {
  const Status status;

  Registration(Status status) : status(status) {}

  void Accept(Visitor &visitor) override { visitor.VisitRegistration(this); }
};

struct Info : public Expr {
public:
  struct Score {
    enum Type { CP, MATE, LOWER_BOUND, UPPER_BOUND };

    Type type;
    int value;
  };

  struct Currline {
    int cpunr;
    std::vector<std::string_view> moves;
  };

  int depth{0};
  int seldepth{0};
  int multipv{0};
  Score score;
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
  Currline currline;

  std::string_view string;

  void Accept(Visitor &visitor) override { visitor.VisitInfo(this); }
};

struct Option : public Expr {
  using Value = std::variant<bool, int, std::string_view>;

  OptionType type;
  std::string id;
  Value def4ult;
  int min = 0;
  int max = -1;
  std::vector<std::string_view> vars;

  void Accept(Visitor &visitor) override { visitor.VisitOption(this); }
};

} // namespace expr
} // namespace uci
#endif
