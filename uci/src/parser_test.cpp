#include <cstdint>
#include <memory>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <uci/command.hpp>
#include <uci/parser.hpp>
#include <uci/scanner.hpp>
#include <uci/types.hpp>

using namespace uci;

struct VisitorMock : public Visitor {
  MOCK_METHOD(void, VisitInput, (command::Input *), (override));
  MOCK_METHOD(void, VisitDebug, (command::Debug *), (override));
  MOCK_METHOD(void, VisitPosition, (command::Position *), (override));
  MOCK_METHOD(void, VisitGo, (command::Go *), (override));
  MOCK_METHOD(void, VisitSetOption, (command::SetOption *), (override));
  MOCK_METHOD(void, VisitRegister, (command::Register *), (override));
  MOCK_METHOD(void, VisitID, (command::ID *), (override));
  MOCK_METHOD(void, VisitBestMove, (command::BestMove *), (override));
  MOCK_METHOD(void, VisitCopyProtection, (command::CopyProtection *),
              (override));
  MOCK_METHOD(void, VisitRegistration, (command::Registration *), (override));
  MOCK_METHOD(void, VisitInfo, (command::Info *), (override));
  MOCK_METHOD(void, VisitOption, (command::Option *), (override));
};

class UCIParserTestSuite : public testing::Test {
 protected:
  VisitorMock mock_;
};

TEST_F(UCIParserTestSuite, TestParseUCIInput) {
  Tokens tokens;
  std::unique_ptr<command::Input> command;

  TOKENIZE(tokens, "uci");
  PARSE(command, tokens);

  ASSERT_NE(command.get(), nullptr);
  ASSERT_EQ(command->input, "uci");

  EXPECT_CALL(mock_, VisitInput(testing::Eq(command.get()))).Times(1);

  command->Accept(mock_);
}

TEST_F(UCIParserTestSuite, TestParseIsReadyInput) {
  Tokens tokens;
  VisitorMock mock;
  std::unique_ptr<command::Input> command;

  TOKENIZE(tokens, "isready");
  PARSE(command, tokens);

  ASSERT_NE(command.get(), nullptr);
  ASSERT_EQ(command->input, "isready");

  EXPECT_CALL(mock_, VisitInput(testing::Eq(command.get()))).Times(1);

  command->Accept(mock_);
}

TEST_F(UCIParserTestSuite, TestParseUCINewGameInput) {
  Tokens tokens;
  VisitorMock mock;
  std::unique_ptr<command::Input> command;

  TOKENIZE(tokens, "ucinewgame");
  PARSE(command, tokens);

  ASSERT_NE(command.get(), nullptr);
  ASSERT_EQ(command->input, "ucinewgame");

  EXPECT_CALL(mock_, VisitInput(testing::Eq(command.get()))).Times(1);

  command->Accept(mock_);
}

TEST_F(UCIParserTestSuite, TestParseStopInput) {
  Tokens tokens;
  VisitorMock mock;
  std::unique_ptr<command::Input> command;

  TOKENIZE(tokens, "stop");
  PARSE(command, tokens);

  ASSERT_NE(command.get(), nullptr);
  ASSERT_EQ(command->input, "stop");

  EXPECT_CALL(mock_, VisitInput(testing::Eq(command.get()))).Times(1);

  command->Accept(mock_);
}

TEST_F(UCIParserTestSuite, TestParsePonderHitInput) {
  Tokens tokens;
  VisitorMock mock;
  std::unique_ptr<command::Input> command;

  TOKENIZE(tokens, "ponderhit");
  PARSE(command, tokens);

  ASSERT_NE(command.get(), nullptr);
  ASSERT_EQ(command->input, "ponderhit");

  EXPECT_CALL(mock_, VisitInput(testing::Eq(command.get()))).Times(1);

  command->Accept(mock_);
}

TEST_F(UCIParserTestSuite, TestParseQuitInput) {
  Tokens tokens;
  VisitorMock mock;
  std::unique_ptr<command::Input> command;

  TOKENIZE(tokens, "quit");
  PARSE(command, tokens);

  ASSERT_NE(command.get(), nullptr);
  ASSERT_EQ(command->input, "quit");

  EXPECT_CALL(mock_, VisitInput(testing::Eq(command.get()))).Times(1);

  command->Accept(mock_);
}

TEST_F(UCIParserTestSuite, TestParseDebugInput) {
  Tokens tokens;
  VisitorMock mock;
  std::unique_ptr<command::Debug> command;

  TOKENIZE(tokens, "debug on");
  PARSE(command, tokens);

  ASSERT_NE(command.get(), nullptr);
  ASSERT_TRUE(command->value);

  TOKENIZE(tokens, "debug off");
  PARSE(command, tokens);

  ASSERT_NE(command.get(), nullptr);
  ASSERT_FALSE(command->value);

  EXPECT_CALL(mock_, VisitDebug(testing::Eq(command.get()))).Times(1);

  command->Accept(mock_);
}

TEST_F(UCIParserTestSuite, TestParsePositionInput) {
  Tokens tokens;
  VisitorMock mock;
  command::Position *position;
  std::unique_ptr<command::Position> command;

  TOKENIZE(tokens, "position startpos");
  PARSE(command, tokens);

  ASSERT_NE(command.get(), nullptr);
  ASSERT_EQ(command->input, "startpos");
  ASSERT_EQ(command->moves.size(), 0);

  TOKENIZE(tokens, "position startpos moves e2e4 c7c5");
  PARSE(command, tokens);

  ASSERT_NE(command.get(), nullptr);
  ASSERT_EQ(command->input, "startpos");
  ASSERT_EQ(command->moves.size(), 2);
  ASSERT_EQ(command->moves[0], "e2e4");
  ASSERT_EQ(command->moves[1], "c7c5");

  TOKENIZE(
      tokens,
      "position fen 1r2k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w k "
      "- 0 1 moves e2e4 c7c5");
  PARSE(command, tokens);

  ASSERT_NE(command.get(), nullptr);
  ASSERT_EQ(command->input,
            "1r2k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w k - 0 1");
  ASSERT_EQ(command->moves.size(), 2);
  ASSERT_EQ(command->moves[0], "e2e4");
  ASSERT_EQ(command->moves[1], "c7c5");

  EXPECT_CALL(mock_, VisitPosition(testing::Eq(command.get()))).Times(1);

  command->Accept(mock_);
}

TEST_F(UCIParserTestSuite, TestParseGoInput) {
  Tokens tokens;
  VisitorMock mock;
  std::unique_ptr<command::Go> command;

  TOKENIZE(tokens, "go");
  PARSE(command, tokens);

  ASSERT_NE(command.get(), nullptr);
  ASSERT_EQ(command->ponder, false);

  TOKENIZE(tokens, "go ponder wsec 1 infinite bsec 2");
  PARSE(command, tokens);

  ASSERT_NE(command, nullptr);
  ASSERT_EQ(command->ponder, true);
  ASSERT_EQ(command->infinite, true);
  ASSERT_EQ(command->wsec, 1);
  ASSERT_EQ(command->bsec, 2);

  TOKENIZE(tokens, "go infinite searchmoves e2e4 d2d4");
  PARSE(command, tokens);

  ASSERT_NE(command, nullptr);
  ASSERT_EQ(command->infinite, true);
  ASSERT_EQ(command->searchmoves.size(), 2);
  ASSERT_EQ(command->searchmoves[0], "e2e4");
  ASSERT_EQ(command->searchmoves[1], "d2d4");

  EXPECT_CALL(mock_, VisitGo(testing::Eq(command.get()))).Times(1);

  command->Accept(mock_);
}

TEST_F(UCIParserTestSuite, TestParseSetOptionInput) {
  Tokens tokens;
  VisitorMock mock;
  std::unique_ptr<command::SetOption> command;

  TOKENIZE(tokens, "setoption name Threads value 40");
  PARSE(command, tokens);

  ASSERT_NE(command.get(), nullptr);
  ASSERT_EQ(command->id, "Threads");
  ASSERT_EQ(std::get<std::int64_t>(command->value), 40);

  TOKENIZE(tokens, "setoption name UCI_AnalyseMode value true");
  PARSE(command, tokens);

  ASSERT_NE(command.get(), nullptr);
  ASSERT_EQ(command->id, "UCI_AnalyseMode");
  ASSERT_TRUE(std::get<bool>(command->value));

  EXPECT_CALL(mock_, VisitSetOption(testing::Eq(command.get()))).Times(1);

  command->Accept(mock_);
}

TEST_F(UCIParserTestSuite, TestParseRegisterInput) {
  Tokens tokens;
  VisitorMock mock;
  std::unique_ptr<command::Register> command;

  TOKENIZE(tokens, "register later");
  PARSE(command, tokens);

  ASSERT_NE(command.get(), nullptr);
  ASSERT_TRUE(command->later);

  TOKENIZE(tokens, "register name Stefan MK code 4359874324");
  PARSE(command, tokens);

  ASSERT_NE(command, nullptr);
  ASSERT_FALSE(command->later);
  ASSERT_EQ(command->name, "Stefan MK");
  ASSERT_EQ(command->code, "4359874324");

  EXPECT_CALL(mock_, VisitRegister(testing::Eq(command.get()))).Times(1);

  command->Accept(mock_);
}

TEST_F(UCIParserTestSuite, TestParseUCIOkInput) {
  Tokens tokens;
  std::unique_ptr<command::Input> command;

  TOKENIZE(tokens, "uciok");
  PARSE(command, tokens);

  ASSERT_NE(command.get(), nullptr);
  ASSERT_EQ(command->input, "uciok");

  EXPECT_CALL(mock_, VisitInput(testing::Eq(command.get()))).Times(1);

  command->Accept(mock_);
}

TEST_F(UCIParserTestSuite, TestParseReadyOkInput) {
  Tokens tokens;
  std::unique_ptr<command::Input> command;

  TOKENIZE(tokens, "readyok");
  PARSE(command, tokens);

  ASSERT_NE(command.get(), nullptr);
  ASSERT_EQ(command->input, "readyok");

  EXPECT_CALL(mock_, VisitInput(testing::Eq(command.get()))).Times(1);

  command->Accept(mock_);
}

TEST_F(UCIParserTestSuite, TestParseIDInput) {
  Tokens tokens;
  std::unique_ptr<command::ID> command;

  TOKENIZE(tokens, "id name Stockfish 17.1");
  PARSE(command, tokens);

  ASSERT_NE(command.get(), nullptr);
  ASSERT_EQ(command->type, command::ID::Type::NAME);
  ASSERT_EQ(command->value, "Stockfish 17.1");

  TOKENIZE(tokens, "id author the Stockfish developers (see AUTHORS file)");
  PARSE(command, tokens);

  ASSERT_NE(command.get(), nullptr);
  ASSERT_EQ(command->type, command::ID::Type::AUTHOR);
  ASSERT_EQ(command->value, "the Stockfish developers (see AUTHORS file)");

  EXPECT_CALL(mock_, VisitID(testing::Eq(command.get()))).Times(1);

  command->Accept(mock_);
}

TEST_F(UCIParserTestSuite, TestParseBestMoveInput) {
  Tokens tokens;
  std::unique_ptr<command::BestMove> command;

  TOKENIZE(tokens, "bestmove c7c5");
  PARSE(command, tokens);

  ASSERT_NE(command.get(), nullptr);
  ASSERT_EQ(command->move, "c7c5");
  ASSERT_EQ(command->ponder, "");

  TOKENIZE(tokens, "bestmove e4e5 ponder b1c3");
  PARSE(command, tokens);

  ASSERT_NE(command.get(), nullptr);
  ASSERT_EQ(command->move, "e4e5");
  ASSERT_EQ(command->ponder, "b1c3");

  EXPECT_CALL(mock_, VisitBestMove(testing::Eq(command.get()))).Times(1);

  command->Accept(mock_);
}

TEST_F(UCIParserTestSuite, TestParseCopyProctectionInput) {
  Tokens tokens;
  std::unique_ptr<command::CopyProtection> command;

  TOKENIZE(tokens, "copyprotection checking");
  PARSE(command, tokens);

  ASSERT_NE(command.get(), nullptr);
  ASSERT_EQ(command->status, Status::CHECKING);

  TOKENIZE(tokens, "copyprotection ok");
  PARSE(command, tokens);

  ASSERT_NE(command.get(), nullptr);
  ASSERT_EQ(command->status, Status::OK);

  TOKENIZE(tokens, "copyprotection error");
  PARSE(command, tokens);

  ASSERT_NE(command.get(), nullptr);
  ASSERT_EQ(command->status, Status::ERROR);

  EXPECT_CALL(mock_, VisitCopyProtection(testing::Eq(command.get()))).Times(1);

  command->Accept(mock_);
}

TEST_F(UCIParserTestSuite, TestParseRegistrationInput) {
  Tokens tokens;
  std::unique_ptr<command::Registration> command;

  TOKENIZE(tokens, "registration checking");
  PARSE(command, tokens);

  ASSERT_NE(command.get(), nullptr);
  ASSERT_EQ(command->status, Status::CHECKING);

  TOKENIZE(tokens, "registration ok");
  PARSE(command, tokens);

  ASSERT_NE(command.get(), nullptr);
  ASSERT_EQ(command->status, Status::OK);

  TOKENIZE(tokens, "registration error");
  PARSE(command, tokens);

  ASSERT_NE(command.get(), nullptr);
  ASSERT_EQ(command->status, Status::ERROR);

  EXPECT_CALL(mock_, VisitRegistration(testing::Eq(command.get()))).Times(1);

  command->Accept(mock_);
}

TEST_F(UCIParserTestSuite, TestParseInfoInput) {
  Tokens tokens;
  std::unique_ptr<command::Info> command;

  TOKENIZE(tokens,
           "info depth 8 seldepth 14 multipv 1 score cp -49 upperbound "
           "nodes 7465 nps 248833 hashfull 1 tbhits 0 time 30 "
           "pv c7c5 b1c3 e7e6 g1f3 b8c6 d2d4 c5d4 f3d4 "
           "refutation d1h5 currline 8 c4c5 e2f3 "
           "string NNUE evaluation using nn-1c0000000000.nnue "
           "(133MiB, (22528, 3072, 15, 32, 1))");

  PARSE(command, tokens);

  ASSERT_NE(command.get(), nullptr);
  ASSERT_EQ(command->depth, 8);
  ASSERT_EQ(command->seldepth, 14);
  ASSERT_EQ(command->multipv, 1);
  ASSERT_EQ(command->score->type, command::Info::Score::CP);
  ASSERT_EQ(command->score->value, -49);
  ASSERT_TRUE(command->score->upperbound);
  ASSERT_EQ(command->nodes, 7465);
  ASSERT_EQ(command->nps, 248833);
  ASSERT_EQ(command->hashfull, 1);
  ASSERT_EQ(command->tbhits, 0);
  ASSERT_EQ(command->time, 30);
  ASSERT_EQ(command->pv.size(), 8);
  ASSERT_EQ(command->pv[0], "c7c5");
  ASSERT_EQ(command->refutation.size(), 1);
  ASSERT_EQ(command->refutation[0], "d1h5");
  ASSERT_EQ(command->currline->cpunr, 8);
  ASSERT_EQ(command->currline->moves.size(), 2);
  ASSERT_EQ(command->currline->moves[0], "c4c5");
  ASSERT_EQ(command->currline->moves[1], "e2f3");
  ASSERT_EQ(command->string,
            "NNUE evaluation using nn-1c0000000000.nnue (133MiB, "
            "(22528, 3072, 15, 32, 1))");

  EXPECT_CALL(mock_, VisitInfo(testing::Eq(command.get()))).Times(1);

  command->Accept(mock_);
}

TEST_F(UCIParserTestSuite, TestParseOptionInput) {
  Tokens tokens;
  std::unique_ptr<command::Option> command;

  TOKENIZE(tokens,
           "option name Style type combo default Normal var Solid var "
           "Normal var Risky");
  PARSE(command, tokens);

  ASSERT_NE(command.get(), nullptr);
  ASSERT_EQ(command->id, "Style");
  ASSERT_EQ(command->type, OptionType::COMBO);
  ASSERT_EQ(std::get<std::string_view>(command->def4ult), "Normal");
  ASSERT_EQ(command->vars.size(), 3);
  ASSERT_EQ(command->vars[0], "Solid");
  ASSERT_EQ(command->vars[1], "Normal");
  ASSERT_EQ(command->vars[2], "Risky");

  TOKENIZE(tokens, "option name Clear Hash type button");
  PARSE(command, tokens);

  ASSERT_NE(command.get(), nullptr);
  ASSERT_EQ(command->id, "Clear Hash");
  ASSERT_EQ(command->type, OptionType::BUTTON);

  TOKENIZE(tokens, "option name NalimovPath type string default c:\\n");
  PARSE(command, tokens);

  ASSERT_NE(command.get(), nullptr);
  ASSERT_EQ(command->id, "NalimovPath");
  ASSERT_EQ(command->type, OptionType::STRING);
  ASSERT_EQ(std::get<std::string_view>(command->def4ult), "c:\\n");

  TOKENIZE(tokens, "option name Selectivity type spin default 2 min 0 max 4");
  PARSE(command, tokens);

  ASSERT_NE(command.get(), nullptr);
  ASSERT_EQ(command->id, "Selectivity");
  ASSERT_EQ(command->type, OptionType::SPIN);
  ASSERT_EQ(std::get<std::int64_t>(command->def4ult), 2);
  ASSERT_EQ(command->min, 0);
  ASSERT_EQ(command->max, 4);

  TOKENIZE(tokens, "option name Nullmove type check default true");
  PARSE(command, tokens);

  ASSERT_NE(command.get(), nullptr);
  ASSERT_EQ(command->id, "Nullmove");
  ASSERT_EQ(command->type, OptionType::CHECK);
  ASSERT_EQ(std::get<bool>(command->def4ult), true);

  EXPECT_CALL(mock_, VisitOption(testing::Eq(command.get()))).Times(1);

  command->Accept(mock_);
}
