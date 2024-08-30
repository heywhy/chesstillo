#include <array>

#include <chesstillo/board.hpp>
#include <chesstillo/fen.hpp>
#include <chesstillo/position.hpp>
#include <chesstillo/types.hpp>
#include <gtest/gtest.h>

class PositionTestSuite : public ::testing::Test {
protected:
  Position position;

  void SetUp() override { ApplyFen(position, START_FEN); }

  void TearDown() override { position.Reset(); }
};

TEST_F(PositionTestSuite, ApplyPawnMove) {
  Move move(e2, e4, PAWN);
  Bitboard empty_squares = ~position.OccupiedSquares();

  position.Make(move);

  ASSERT_NE(~position.OccupiedSquares(), empty_squares);
  ASSERT_EQ(PositionToFen(position),
            "rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1");
}

TEST_F(PositionTestSuite, ApplyPawnCaptureMove) {
  std::array<std::array<int, 2>, 6> moves = {
      {{e2, e4}, {d7, d5}, {e4, d5}, {c7, c6}, {d2, d4}, {c6, d5}}};

  for (auto [from, to] : moves) {
    Move move = DeduceMove(position, from, to);

    position.Make(move);
  }

  ASSERT_EQ(PositionToFen(position),
            "rnbqkbnr/pp2pppp/8/3p4/3P4/8/PPP2PPP/RNBQKBNR w KQkq - 0 4");
}

TEST_F(PositionTestSuite, MoveKnight) {
  ApplyFen(position, "8/8/4n3/8/3N4/8/8/8 w - - 0 1");

  position.Make({d4, e6, KNIGHT});

  ASSERT_EQ(PositionToFen(position), "8/8/4N3/8/8/8/8/8 b - - 1 1");
}

TEST_F(PositionTestSuite, MoveBishop) {
  ApplyFen(position, "8/8/8/3b4/3B4/8/8/8 w - - 0 1");

  std::array<std::array<int, 2>, 2> moves = {{{d4, b6}, {d5, f7}}};

  for (auto [from, to] : moves) {
    Move move = DeduceMove(position, from, to);

    position.Make(move);
  }

  ASSERT_EQ(PositionToFen(position), "8/5b2/1B6/8/8/8/8/8 w - - 2 2");
}

TEST_F(PositionTestSuite, MoveRook) {
  ApplyFen(position, "8/2r2n2/8/8/3R4/8/8/5R2 w - - 0 1");

  std::array<std::array<int, 2>, 2> moves = {{{f1, f7}, {c7, f7}}};

  for (auto [from, to] : moves) {
    Move move = DeduceMove(position, from, to);

    position.Make(move);
  }

  ASSERT_EQ(PositionToFen(position), "8/5r2/8/8/3R4/8/8/8 w - - 0 2");
}

TEST_F(PositionTestSuite, MoveQueen) {
  ApplyFen(position, "8/8/4r3/1n1N2q1/2Q5/8/8/8 w - - 0 1");

  std::array<std::array<int, 2>, 9> moves = {{
      {c4, b5},
      {e6, e5},
      {b5, f1},
      {e5, d5},
      {f1, e2},
      {g5, h6},
      {e2, e8},
      {h6, a6},
  }};

  for (auto [from, to] : moves) {
    Move move = DeduceMove(position, from, to);

    position.Make(move);
  }

  ASSERT_EQ(PositionToFen(position), "4Q3/8/q7/3r4/8/8/8/8 b - - 5 5");
}

TEST_F(PositionTestSuite, EnPassantCapture) {
  ApplyFen(position,
           "rnbqkbnr/pppp1ppp/8/8/3pP3/8/PPP2PPP/RNBQKBNR w KQkq - 0 1");

  Move white = DeduceMove(position, c2, c4);

  position.Make(white);

  ASSERT_EQ(PositionToFen(position),
            "rnbqkbnr/pppp1ppp/8/8/2PpP3/8/PP3PPP/RNBQKBNR b KQkq c3 0 1");

  Move black = DeduceMove(position, d4, c3);

  position.Make(black);

  ASSERT_EQ(PositionToFen(position),
            "rnbqkbnr/pppp1ppp/8/8/4P3/2p5/PP3PPP/RNBQKBNR w KQkq - 0 2");

  position.Undo(black);

  ASSERT_EQ(PositionToFen(position),
            "rnbqkbnr/pppp1ppp/8/8/2PpP3/8/PP3PPP/RNBQKBNR b KQkq c3 0 1");

  position.Undo(white);

  ApplyFen(position,
           "rnbqkbnr/pppp1ppp/8/8/3pP3/8/PPP2PPP/RNBQKBNR w KQkq - 0 1");
}

TEST_F(PositionTestSuite, UpdateCastlingRights) {
  ApplyFen(position,
           "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -");

  Move castle_right = DeduceMove(position, e1, g1);

  position.Make(castle_right);

  ASSERT_EQ(
      PositionToFen(position),
      "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R4RK1 b kq - 1 1");

  position.Undo(castle_right);

  ASSERT_EQ(
      PositionToFen(position),
      "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1");

  Move castle_left = DeduceMove(position, e1, c1);

  position.Make(castle_left);

  ASSERT_EQ(
      PositionToFen(position),
      "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/2KR3R b kq - 1 1");

  position.Undo(castle_left);

  ASSERT_EQ(
      PositionToFen(position),
      "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1");
}
