#include <uci/expr.hpp>

using namespace uci;

void expr::Command::Accept(Visitor &visitor) { visitor.VisitCommand(this); }

void expr::Position::Accept(Visitor &visitor) { visitor.VisitPosition(this); }

void expr::Go::Accept(Visitor &visitor) { visitor.VisitGo(this); }

void expr::SetOption::Accept(Visitor &visitor) { visitor.VisitSetOption(this); }

void expr::Register::Accept(Visitor &visitor) { visitor.VisitRegister(this); }
