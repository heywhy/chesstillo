#include <gtest/gtest.h>

#include "engine/constants.hpp"
#include "engine/position.hpp"
#include "engine/types.hpp"

using namespace engine;

TEST(FenTestSuite, TestMakePositionFromFen) {
  Position position = Position::FromFen(kStartPos);

  ASSERT_EQ(position.ToFen(), kStartPos);
}

TEST(FenTestSuite, TestCastlingRightsDefinedInFenIsMaintained) {
  Position position = Position::FromFen(
      "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w Kq - 0 1");

  ASSERT_TRUE(position.CanCastle(position::CASTLE_W_KING_SIDE));
  ASSERT_FALSE(position.CanCastle(position::CASTLE_W_QUEEN_SIDE));
  ASSERT_FALSE(position.CanCastle(position::CASTLE_B_KING_SIDE));
  ASSERT_TRUE(position.CanCastle(position::CASTLE_B_QUEEN_SIDE));
}

TEST(FenTestSuite, TestIgnoreCastlingRightsIfMissing) {
  Position position = Position::FromFen(
      "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w - - 0 1");

  ASSERT_FALSE(position.CanCastle(position::CASTLE_W_KING_SIDE));
  ASSERT_FALSE(position.CanCastle(position::CASTLE_W_QUEEN_SIDE));
  ASSERT_FALSE(position.CanCastle(position::CASTLE_B_KING_SIDE));
  ASSERT_FALSE(position.CanCastle(position::CASTLE_B_QUEEN_SIDE));
}

TEST(FenTestSuite, TestSetEnPassantSquare) {
  Position position;

  ASSERT_EQ(position.EnPassantSquare(), kEmpty);

  position = Position::FromFen(
      "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w - e3 0 1");

  ASSERT_TRUE(position.EnPassantSquare());
}

TEST(FenTestSuite, TestMakeFenFromPosition) {
  Position position = Position::FromFen(kStartPos);

  ASSERT_EQ(position.ToFen(), kStartPos);

  Move white_move(e2, e4, PAWN);
  Move black_move(c7, c5, PAWN);

  position.Make(white_move);
  position.Make(black_move);

  ASSERT_EQ(position.ToFen(),
            "rnbqkbnr/pp1ppppp/8/2p5/4P3/8/PPPP1PPP/RNBQKBNR w KQkq c6 0 2");
}

TEST(FenTestSuite, TestRepresentEnPassantSquareInFen) {
  const char *fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w - e3 0 1";
  Position position = Position::FromFen(fen);

  ASSERT_EQ(position.ToFen(), fen);

  position = Position::FromFen(kStartPos);

  ASSERT_EQ(position.ToFen(), kStartPos);
}

TEST(FenTestSuite, TestMovesClockAndCounter) {
  const char *fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w - e3 50 97";
  Position position = Position::FromFen(fen);

  ASSERT_EQ(position.ToFen(), fen);

  position = Position::FromFen(kStartPos);

  ASSERT_EQ(position.ToFen(), kStartPos);
}
