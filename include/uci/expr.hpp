#ifndef UCI_EXPRESSIONS_HPP
#define UCI_EXPRESSIONS_HPP

#include <string>
#include <string_view>
#include <vector>

namespace uci {
namespace expr {
struct Debug;
struct SetOption;
struct Register;
struct Position;
struct Go;

struct Command;
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
};

namespace expr {
struct Command : public Expr {
  const std::string_view &input;

  Command(const std::string_view &command) : input(command) {};

  void Accept(Visitor &) override;
};

struct Position : public Expr {
  const std::string input;
  std::vector<std::string_view> moves;

  Position(const std::string_view &pos) : input(pos) {};

  void Accept(Visitor &) override;
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

  void Accept(Visitor &) override;
};

struct SetOption : public Expr {
  const std::string_view &id;
  std::string value;

  SetOption(const std::string_view &id) : id(id) {}

  void Accept(Visitor &) override;
};

struct Register : public Expr {
  bool later = false;
  std::string name;
  std::string_view code;

  void Accept(Visitor &) override;
};

} // namespace expr
} // namespace uci
#endif
