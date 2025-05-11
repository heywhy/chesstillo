#include <memory>

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <uci/expr.hpp>
#include <uci/parser.hpp>
#include <uci/scanner.hpp>

using namespace uci;

struct VisitorMock : public Visitor {
  MOCK_METHOD(void, VisitCommand, (expr::Command *), (override));
  MOCK_METHOD(void, VisitPosition, (expr::Position *), (override));
  MOCK_METHOD(void, VisitGo, (expr::Go *), (override));
  MOCK_METHOD(void, VisitSetOption, (expr::SetOption *), (override));
  MOCK_METHOD(void, VisitRegister, (expr::Register *), (override));
};

class UCIParserTestSuite : public testing::Test {
protected:
  VisitorMock mock_;
};

TEST_F(UCIParserTestSuite, TestParseUCICommand) {
  Tokens tokens;
  std::unique_ptr<expr::Command> expr;

  TOKENIZE(tokens, "uci");
  PARSE(expr, tokens);

  ASSERT_NE(expr.get(), nullptr);
  ASSERT_EQ(expr->input, "uci");

  EXPECT_CALL(mock_, VisitCommand(testing::Eq(expr.get()))).Times(1);

  expr->Accept(mock_);
}

TEST_F(UCIParserTestSuite, TestParseIsReadyCommand) {
  Tokens tokens;
  VisitorMock mock;
  std::unique_ptr<expr::Command> expr;

  TOKENIZE(tokens, "isready");
  PARSE(expr, tokens);

  ASSERT_NE(expr.get(), nullptr);
  ASSERT_EQ(expr->input, "isready");

  EXPECT_CALL(mock_, VisitCommand(testing::Eq(expr.get()))).Times(1);

  expr->Accept(mock_);
}

TEST_F(UCIParserTestSuite, TestParseUCINewGameCommand) {
  Tokens tokens;
  VisitorMock mock;
  std::unique_ptr<expr::Command> expr;

  TOKENIZE(tokens, "ucinewgame");
  PARSE(expr, tokens);

  ASSERT_NE(expr.get(), nullptr);
  ASSERT_EQ(expr->input, "ucinewgame");

  EXPECT_CALL(mock_, VisitCommand(testing::Eq(expr.get()))).Times(1);

  expr->Accept(mock_);
}

TEST_F(UCIParserTestSuite, TestParseStopCommand) {
  Tokens tokens;
  VisitorMock mock;
  std::unique_ptr<expr::Command> expr;

  TOKENIZE(tokens, "stop");
  PARSE(expr, tokens);

  ASSERT_NE(expr.get(), nullptr);
  ASSERT_EQ(expr->input, "stop");

  EXPECT_CALL(mock_, VisitCommand(testing::Eq(expr.get()))).Times(1);

  expr->Accept(mock_);
}

TEST_F(UCIParserTestSuite, TestParsePonderHitCommand) {
  Tokens tokens;
  VisitorMock mock;
  std::unique_ptr<expr::Command> expr;

  TOKENIZE(tokens, "ponderhit");
  PARSE(expr, tokens);

  ASSERT_NE(expr.get(), nullptr);
  ASSERT_EQ(expr->input, "ponderhit");

  EXPECT_CALL(mock_, VisitCommand(testing::Eq(expr.get()))).Times(1);

  expr->Accept(mock_);
}

TEST_F(UCIParserTestSuite, TestParseQuitCommand) {
  Tokens tokens;
  VisitorMock mock;
  std::unique_ptr<expr::Command> expr;

  TOKENIZE(tokens, "quit");
  PARSE(expr, tokens);

  ASSERT_NE(expr.get(), nullptr);
  ASSERT_EQ(expr->input, "quit");

  EXPECT_CALL(mock_, VisitCommand(testing::Eq(expr.get()))).Times(1);

  expr->Accept(mock_);
}

TEST_F(UCIParserTestSuite, TestParsePositionCommand) {
  Tokens tokens;
  VisitorMock mock;
  expr::Position *position;
  std::unique_ptr<expr::Position> expr;

  TOKENIZE(tokens, "position startpos");
  PARSE(expr, tokens);

  ASSERT_NE(expr.get(), nullptr);
  ASSERT_EQ(expr->input, "startpos");
  ASSERT_EQ(expr->moves.size(), 0);

  TOKENIZE(tokens, "position startpos moves e2e4 c7c5");
  PARSE(expr, tokens);

  ASSERT_NE(expr.get(), nullptr);
  ASSERT_EQ(expr->input, "startpos");
  ASSERT_EQ(expr->moves.size(), 2);
  ASSERT_EQ(expr->moves[0], "e2e4");
  ASSERT_EQ(expr->moves[1], "c7c5");

  TOKENIZE(
      tokens,
      "position fen 1r2k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w k "
      "- 0 1 moves e2e4 c7c5");
  PARSE(expr, tokens);

  ASSERT_NE(expr.get(), nullptr);
  ASSERT_EQ(expr->input,
            "1r2k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w k - 0 1");
  ASSERT_EQ(expr->moves.size(), 2);
  ASSERT_EQ(expr->moves[0], "e2e4");
  ASSERT_EQ(expr->moves[1], "c7c5");

  EXPECT_CALL(mock_, VisitPosition(testing::Eq(expr.get()))).Times(1);

  expr->Accept(mock_);
}

TEST_F(UCIParserTestSuite, TestParseGoCommand) {
  Tokens tokens;
  VisitorMock mock;
  std::unique_ptr<expr::Go> expr;

  TOKENIZE(tokens, "go");
  PARSE(expr, tokens);

  ASSERT_NE(expr.get(), nullptr);
  ASSERT_EQ(expr->ponder, false);

  TOKENIZE(tokens, "go ponder wsec 1 infinite bsec 2");
  PARSE(expr, tokens);

  ASSERT_NE(expr, nullptr);
  ASSERT_EQ(expr->ponder, true);
  ASSERT_EQ(expr->infinite, true);
  ASSERT_EQ(expr->wsec, 1);
  ASSERT_EQ(expr->bsec, 2);

  EXPECT_CALL(mock_, VisitGo(testing::Eq(expr.get()))).Times(1);

  expr->Accept(mock_);
}

TEST_F(UCIParserTestSuite, TestParseSetOptionCommand) {
  Tokens tokens;
  VisitorMock mock;
  std::unique_ptr<expr::SetOption> expr;

  TOKENIZE(tokens, "setoption name Threads value 40");
  PARSE(expr, tokens);

  ASSERT_NE(expr.get(), nullptr);
  ASSERT_EQ(expr->id, "Threads");
  ASSERT_EQ(expr->value, "40");

  EXPECT_CALL(mock_, VisitSetOption(testing::Eq(expr.get()))).Times(1);

  expr->Accept(mock_);
}

TEST_F(UCIParserTestSuite, TestParseRegisterCommand) {
  Tokens tokens;
  VisitorMock mock;
  std::unique_ptr<expr::Register> expr;

  TOKENIZE(tokens, "register later");
  PARSE(expr, tokens);

  ASSERT_NE(expr.get(), nullptr);
  ASSERT_TRUE(expr->later);

  TOKENIZE(tokens, "register name Stefan MK code 4359874324");
  PARSE(expr, tokens);

  ASSERT_NE(expr, nullptr);
  ASSERT_FALSE(expr->later);
  ASSERT_EQ(expr->name, "Stefan MK");
  ASSERT_EQ(expr->code, "4359874324");

  EXPECT_CALL(mock_, VisitRegister(testing::Eq(expr.get()))).Times(1);

  expr->Accept(mock_);
}
