#include "uci/types.hpp"
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
  MOCK_METHOD(void, VisitID, (expr::ID *), (override));
  MOCK_METHOD(void, VisitBestMove, (expr::BestMove *), (override));
  MOCK_METHOD(void, VisitCopyProtection, (expr::CopyProtection *), (override));
  MOCK_METHOD(void, VisitRegistration, (expr::Registration *), (override));
  MOCK_METHOD(void, VisitInfo, (expr::Info *), (override));
  MOCK_METHOD(void, VisitOption, (expr::Option *), (override));
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

TEST_F(UCIParserTestSuite, TestParseUCIOkCommand) {
  Tokens tokens;
  std::unique_ptr<expr::Command> expr;

  TOKENIZE(tokens, "uciok");
  PARSE(expr, tokens);

  ASSERT_NE(expr.get(), nullptr);
  ASSERT_EQ(expr->input, "uciok");

  EXPECT_CALL(mock_, VisitCommand(testing::Eq(expr.get()))).Times(1);

  expr->Accept(mock_);
}

TEST_F(UCIParserTestSuite, TestParseReadyOkCommand) {
  Tokens tokens;
  std::unique_ptr<expr::Command> expr;

  TOKENIZE(tokens, "readyok");
  PARSE(expr, tokens);

  ASSERT_NE(expr.get(), nullptr);
  ASSERT_EQ(expr->input, "readyok");

  EXPECT_CALL(mock_, VisitCommand(testing::Eq(expr.get()))).Times(1);

  expr->Accept(mock_);
}

TEST_F(UCIParserTestSuite, TestParseIDCommand) {
  Tokens tokens;
  std::unique_ptr<expr::ID> expr;

  TOKENIZE(tokens, "id name Stockfish 17.1");
  PARSE(expr, tokens);

  ASSERT_NE(expr.get(), nullptr);
  ASSERT_EQ(expr->type, expr::ID::Type::NAME);
  ASSERT_EQ(expr->value, "Stockfish 17.1");

  TOKENIZE(tokens, "id author the Stockfish developers (see AUTHORS file)");
  PARSE(expr, tokens);

  ASSERT_NE(expr.get(), nullptr);
  ASSERT_EQ(expr->type, expr::ID::Type::AUTHOR);
  ASSERT_EQ(expr->value, "the Stockfish developers (see AUTHORS file)");

  EXPECT_CALL(mock_, VisitID(testing::Eq(expr.get()))).Times(1);

  expr->Accept(mock_);
}

TEST_F(UCIParserTestSuite, TestParseBestMoveCommand) {
  Tokens tokens;
  std::unique_ptr<expr::BestMove> expr;

  TOKENIZE(tokens, "bestmove c7c5");
  PARSE(expr, tokens);

  ASSERT_NE(expr.get(), nullptr);
  ASSERT_EQ(expr->move, "c7c5");
  ASSERT_EQ(expr->ponder, "");

  TOKENIZE(tokens, "bestmove e4e5 ponder b1c3");
  PARSE(expr, tokens);

  ASSERT_NE(expr.get(), nullptr);
  ASSERT_EQ(expr->move, "e4e5");
  ASSERT_EQ(expr->ponder, "b1c3");

  EXPECT_CALL(mock_, VisitBestMove(testing::Eq(expr.get()))).Times(1);

  expr->Accept(mock_);
}

TEST_F(UCIParserTestSuite, TestParseCopyProctectionCommand) {
  Tokens tokens;
  std::unique_ptr<expr::CopyProtection> expr;

  TOKENIZE(tokens, "copyprotection checking");
  PARSE(expr, tokens);

  ASSERT_NE(expr.get(), nullptr);
  ASSERT_EQ(expr->status, Status::CHECKING);

  TOKENIZE(tokens, "copyprotection ok");
  PARSE(expr, tokens);

  ASSERT_NE(expr.get(), nullptr);
  ASSERT_EQ(expr->status, Status::OK);

  TOKENIZE(tokens, "copyprotection error");
  PARSE(expr, tokens);

  ASSERT_NE(expr.get(), nullptr);
  ASSERT_EQ(expr->status, Status::ERROR);

  EXPECT_CALL(mock_, VisitCopyProtection(testing::Eq(expr.get()))).Times(1);

  expr->Accept(mock_);
}

TEST_F(UCIParserTestSuite, TestParseRegistrationCommand) {
  Tokens tokens;
  std::unique_ptr<expr::Registration> expr;

  TOKENIZE(tokens, "registration checking");
  PARSE(expr, tokens);

  ASSERT_NE(expr.get(), nullptr);
  ASSERT_EQ(expr->status, Status::CHECKING);

  TOKENIZE(tokens, "registration ok");
  PARSE(expr, tokens);

  ASSERT_NE(expr.get(), nullptr);
  ASSERT_EQ(expr->status, Status::OK);

  TOKENIZE(tokens, "registration error");
  PARSE(expr, tokens);

  ASSERT_NE(expr.get(), nullptr);
  ASSERT_EQ(expr->status, Status::ERROR);

  EXPECT_CALL(mock_, VisitRegistration(testing::Eq(expr.get()))).Times(1);

  expr->Accept(mock_);
}

TEST_F(UCIParserTestSuite, TestParseInfoCommand) {
  Tokens tokens;
  std::unique_ptr<expr::Info> expr;

  TOKENIZE(tokens, "info depth 8 seldepth 14 multipv 1 score cp -49 "
                   "nodes 7465 nps 248833 hashfull 1 tbhits 0 time 30 "
                   "pv c7c5 b1c3 e7e6 g1f3 b8c6 d2d4 c5d4 f3d4 "
                   "refutation d1h5 currline 8 c4c5 e2f3 "
                   "string NNUE evaluation using nn-1c0000000000.nnue "
                   "(133MiB, (22528, 3072, 15, 32, 1))");

  PARSE(expr, tokens);

  ASSERT_NE(expr.get(), nullptr);
  ASSERT_EQ(expr->depth, 8);
  ASSERT_EQ(expr->seldepth, 14);
  ASSERT_EQ(expr->multipv, 1);
  ASSERT_EQ(expr->score.type, expr::Info::Score::CP);
  ASSERT_EQ(expr->score.value, -49);
  ASSERT_EQ(expr->nodes, 7465);
  ASSERT_EQ(expr->nps, 248833);
  ASSERT_EQ(expr->hashfull, 1);
  ASSERT_EQ(expr->tbhits, 0);
  ASSERT_EQ(expr->time, 30);
  ASSERT_EQ(expr->pv.size(), 8);
  ASSERT_EQ(expr->pv[0], "c7c5");
  ASSERT_EQ(expr->refutation.size(), 1);
  ASSERT_EQ(expr->refutation[0], "d1h5");
  ASSERT_EQ(expr->currline.cpunr, 8);
  ASSERT_EQ(expr->currline.moves.size(), 2);
  ASSERT_EQ(expr->currline.moves[0], "c4c5");
  ASSERT_EQ(expr->currline.moves[1], "e2f3");
  ASSERT_EQ(expr->string, "NNUE evaluation using nn-1c0000000000.nnue (133MiB, "
                          "(22528, 3072, 15, 32, 1))");

  EXPECT_CALL(mock_, VisitInfo(testing::Eq(expr.get()))).Times(1);

  expr->Accept(mock_);
}

TEST_F(UCIParserTestSuite, TestParseOptionCommand) {
  Tokens tokens;
  std::unique_ptr<expr::Option> expr;

  TOKENIZE(tokens, "option name Style type combo default Normal var Solid var "
                   "Normal var Risky");
  PARSE(expr, tokens);

  ASSERT_NE(expr.get(), nullptr);
  ASSERT_EQ(expr->id, "Style");
  ASSERT_EQ(expr->type, OptionType::COMBO);
  ASSERT_EQ(std::get<std::string_view>(expr->def4ult), "Normal");
  ASSERT_EQ(expr->vars.size(), 3);
  ASSERT_EQ(expr->vars[0], "Solid");
  ASSERT_EQ(expr->vars[1], "Normal");
  ASSERT_EQ(expr->vars[2], "Risky");

  TOKENIZE(tokens, "option name Clear Hash type button");
  PARSE(expr, tokens);

  ASSERT_NE(expr.get(), nullptr);
  ASSERT_EQ(expr->id, "Clear Hash");
  ASSERT_EQ(expr->type, OptionType::BUTTON);

  TOKENIZE(tokens, "option name NalimovPath type string default c:\\n");
  PARSE(expr, tokens);

  ASSERT_NE(expr.get(), nullptr);
  ASSERT_EQ(expr->id, "NalimovPath");
  ASSERT_EQ(expr->type, OptionType::STRING);
  ASSERT_EQ(std::get<std::string_view>(expr->def4ult), "c:\\n");

  TOKENIZE(tokens, "option name Selectivity type spin default 2 min 0 max 4");
  PARSE(expr, tokens);

  ASSERT_NE(expr.get(), nullptr);
  ASSERT_EQ(expr->id, "Selectivity");
  ASSERT_EQ(expr->type, OptionType::SPIN);
  ASSERT_EQ(std::get<int>(expr->def4ult), 2);
  ASSERT_EQ(expr->min, 0);
  ASSERT_EQ(expr->max, 4);

  TOKENIZE(tokens, "option name Nullmove type check default true");
  PARSE(expr, tokens);

  ASSERT_NE(expr.get(), nullptr);
  ASSERT_EQ(expr->id, "Nullmove");
  ASSERT_EQ(expr->type, OptionType::CHECK);
  ASSERT_EQ(std::get<bool>(expr->def4ult), true);

  EXPECT_CALL(mock_, VisitOption(testing::Eq(expr.get()))).Times(1);

  expr->Accept(mock_);
}
