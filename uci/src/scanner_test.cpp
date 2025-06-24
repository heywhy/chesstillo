#include <cstdint>

#include <gtest/gtest.h>

#include <uci/scanner.hpp>
#include <uci/types.hpp>

using namespace uci;

class UCIScannerTestSuite : public testing::Test {};

TEST_F(UCIScannerTestSuite, TestParseCommand) {
  uci::Tokens tokens;

  TOKENIZE(tokens, "uci");

  ASSERT_EQ(tokens.size(), 1);

  ASSERT_EQ(tokens[0].type, TokenType::UCI);
  ASSERT_EQ(tokens[0].lexeme, "uci");
  ASSERT_EQ(std::get<std::string_view>(tokens[0].literal), "uci");
  ASSERT_EQ(tokens[0].col, 1);
  ASSERT_EQ(tokens[0].line, 1);

  TOKENIZE(tokens, "debug on");

  ASSERT_EQ(tokens.size(), 2);

  ASSERT_EQ(tokens[0].type, TokenType::DEBUG);
  ASSERT_EQ(tokens[0].lexeme, "debug");
  ASSERT_EQ(std::get<std::string_view>(tokens[0].literal), "debug");

  ASSERT_EQ(tokens[1].type, TokenType::WORD);
  ASSERT_EQ(tokens[1].lexeme, "on");
  ASSERT_EQ(std::get<std::string_view>(tokens[1].literal), "on");
  ASSERT_EQ(tokens[1].col, 7);

  TOKENIZE(tokens, "   debug     on  ");

  ASSERT_EQ(tokens.size(), 2);

  ASSERT_EQ(tokens[0].type, TokenType::DEBUG);
  ASSERT_EQ(tokens[0].lexeme, "debug");
  ASSERT_EQ(std::get<std::string_view>(tokens[0].literal), "debug");
  ASSERT_EQ(tokens[0].col, 4);

  ASSERT_EQ(tokens[1].type, TokenType::WORD);
  ASSERT_EQ(tokens[1].lexeme, "on");
  ASSERT_EQ(std::get<std::string_view>(tokens[1].literal), "on");
  ASSERT_EQ(tokens[1].col, 14);

  TOKENIZE(tokens, "\t  debug \t  \t\ton\t  ");

  ASSERT_EQ(tokens.size(), 2);

  ASSERT_EQ(tokens[0].type, TokenType::DEBUG);
  ASSERT_EQ(tokens[0].lexeme, "debug");
  ASSERT_EQ(std::get<std::string_view>(tokens[0].literal), "debug");

  ASSERT_EQ(tokens[1].type, TokenType::WORD);
  ASSERT_EQ(tokens[1].lexeme, "on");
  ASSERT_EQ(std::get<std::string_view>(tokens[1].literal), "on");

  TOKENIZE(tokens, "setoption name Threads value 65");

  ASSERT_EQ(tokens.size(), 5);

  ASSERT_EQ(tokens[0].type, TokenType::SET_OPTION);
  ASSERT_EQ(tokens[0].lexeme, "setoption");

  ASSERT_EQ(tokens[4].type, TokenType::NUMBER);
  ASSERT_EQ(tokens[4].lexeme, "65");
  ASSERT_EQ(std::get<std::int64_t>(tokens[4].literal), 65);
}

TEST_F(UCIScannerTestSuite, TestParseFeedback) {
  Tokens tokens;

  TOKENIZE(tokens, "uciok");

  ASSERT_EQ(tokens.size(), 1);

  ASSERT_EQ(tokens[0].type, TokenType::UCI_OK);
  ASSERT_EQ(tokens[0].lexeme, "uciok");
  ASSERT_EQ(std::get<std::string_view>(tokens[0].literal), "uciok");
  ASSERT_EQ(tokens[0].col, 1);
  ASSERT_EQ(tokens[0].line, 1);

  TOKENIZE(tokens, "id name chesstillo");

  ASSERT_EQ(tokens.size(), 3);

  ASSERT_EQ(tokens[0].type, TokenType::ID);
  ASSERT_EQ(tokens[0].lexeme, "id");
  ASSERT_EQ(std::get<std::string_view>(tokens[0].literal), "id");
  ASSERT_EQ(tokens[0].col, 1);
  ASSERT_EQ(tokens[0].line, 1);

  ASSERT_EQ(tokens[1].type, TokenType::WORD);
  ASSERT_EQ(tokens[1].lexeme, "name");
  ASSERT_EQ(std::get<std::string_view>(tokens[1].literal), "name");
  ASSERT_EQ(tokens[1].col, 4);
  ASSERT_EQ(tokens[1].line, 1);

  ASSERT_EQ(tokens[2].type, TokenType::WORD);
  ASSERT_EQ(tokens[2].lexeme, "chesstillo");
  ASSERT_EQ(std::get<std::string_view>(tokens[2].literal), "chesstillo");
  ASSERT_EQ(tokens[2].col, 9);
  ASSERT_EQ(tokens[2].line, 1);

  TOKENIZE(tokens,
           "info depth 8 seldepth 14 multipv 1 score cp -49 "
           "nodes 7465 nps 248833 hashfull 1 tbhits 0 time 30 "
           "pv c7c5 b1c3 e7e6 g1f3 b8c6 d2d4 c5d4 f3d4");

  ASSERT_EQ(tokens.size(), 29);

  ASSERT_EQ(tokens[0].type, TokenType::INFO);
  ASSERT_EQ(tokens[0].lexeme, "info");
  ASSERT_EQ(std::get<std::string_view>(tokens[0].literal), "info");

  ASSERT_EQ(tokens[2].type, TokenType::NUMBER);
  ASSERT_EQ(tokens[2].lexeme, "8");
  ASSERT_EQ(std::get<std::int64_t>(tokens[2].literal), 8);

  ASSERT_EQ(tokens[9].type, TokenType::NUMBER);
  ASSERT_EQ(tokens[9].lexeme, "-49");
  ASSERT_EQ(std::get<std::int64_t>(tokens[9].literal), -49);
}
