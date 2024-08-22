#include <chesstillo/fen.hpp>
#include <chesstillo/position.hpp>
#include <gtest/gtest.h>

TEST(FenTest, ApplyFen) {
  Position position;

  ASSERT_EQ(position.OccupiedSquares(), kEmpty);

  ApplyFen(position, START_FEN);

  ASSERT_NE(position.OccupiedSquares(), kEmpty);
}

TEST(FenTest, MaintainCastlingRights) {
  Position position;

  ApplyFen(position, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w Kq - 0 1");

  ASSERT_TRUE(position.CanCastle(K_WHITE));
  ASSERT_TRUE(position.CanCastle(Q_BLACK));
  ASSERT_FALSE(position.CanCastle(Q_WHITE));
  ASSERT_FALSE(position.CanCastle(K_BLACK));
}

TEST(FenTest, MaintainCastlingRightsIfMissing) {
  Position position;

  ApplyFen(position, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w - - 0 1");

  ASSERT_FALSE(position.CanCastle(K_WHITE));
  ASSERT_FALSE(position.CanCastle(Q_BLACK));
  ASSERT_FALSE(position.CanCastle(Q_WHITE));
  ASSERT_FALSE(position.CanCastle(K_BLACK));
}

TEST(FenTest, ApplyEnPassantSquare) {
  Position position;

  ApplyFen(position, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w - e3 0 1");

  ASSERT_TRUE(position.EnPassantSquare());

  ApplyFen(position, START_FEN);

  ASSERT_FALSE(position.EnPassantSquare());
}

TEST(FenTest, PositionToFen) {
  Position position;

  ApplyFen(position, START_FEN);

  ASSERT_EQ(PositionToFen(position), START_FEN);

  Move white_move(e2, e4, PAWN);
  Move black_move(c7, c5, PAWN);

  position.Make(white_move);
  position.Make(black_move);

  ASSERT_EQ(PositionToFen(position),
            "rnbqkbnr/pp1ppppp/8/2p5/4P3/8/PPPP1PPP/RNBQKBNR w KQkq - 0 2");
}

TEST(FenTest, RepresentEnPassantSquareInPositionFen) {
  Position position;
  const char *fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w - e3 0 1";

  ApplyFen(position, fen);

  ASSERT_EQ(PositionToFen(position), fen);

  ApplyFen(position, START_FEN);

  ASSERT_EQ(PositionToFen(position), START_FEN);
}

TEST(FenTest, ClockMovesInFenAreRespected) {
  Position position;
  const char *fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w - e3 50 97";

  ApplyFen(position, fen);

  ASSERT_EQ(PositionToFen(position), fen);

  ApplyFen(position, START_FEN);

  ASSERT_EQ(PositionToFen(position), START_FEN);
}
