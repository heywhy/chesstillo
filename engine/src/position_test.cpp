#include <array>
#include <cstdint>
#include <utility>

#include <gtest/gtest.h>

#include <engine/board.hpp>
#include <engine/position.hpp>
#include <engine/types.hpp>
#include <engine/utility.hpp>

using namespace engine;

TEST(PositionTestSuite, TestPawnMove) {
  Move move(e2, e4, PAWN);
  Position position = Position::FromFen(kStartPos);

  position.Make(move);

  ASSERT_EQ(position.ToFen(),
            "rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1");
}

TEST(PositionTestSuite, TestPawnCaptureMove) {
  Position position = Position::FromFen(kStartPos);
  std::array<std::pair<uint8_t, uint8_t>, 6> moves = {
      {{e2, e4}, {d7, d5}, {e4, d5}, {c7, c6}, {d2, d4}, {c6, d5}}};

  for (auto [from, to] : moves) {
    Move move = DeduceMove(position, from, to);

    position.Make(move);
  }

  ASSERT_EQ(position.ToFen(),
            "rnbqkbnr/pp2pppp/8/3p4/3P4/8/PPP2PPP/RNBQKBNR w KQkq - 0 4");
}

TEST(PositionTestSuite, TestKnightMove) {
  Move move(d4, e6, KNIGHT);
  Position position = Position::FromFen("8/8/4n3/8/3N4/8/8/8 w - - 0 1");

  position.Make(move);

  ASSERT_EQ(position.ToFen(), "8/8/4N3/8/8/8/8/8 b - - 1 1");
}

TEST(PositionTestSuite, TestBishopMove) {
  Position position = Position::FromFen("8/8/8/3b4/3B4/8/8/8 w - - 0 1");

  std::array<std::array<uint8_t, 2>, 2> moves = {{{d4, b6}, {d5, f7}}};

  for (auto [from, to] : moves) {
    Move move = DeduceMove(position, from, to);

    position.Make(move);
  }

  ASSERT_EQ(position.ToFen(), "8/5b2/1B6/8/8/8/8/8 w - - 2 2");
}

TEST(PositionTestSuite, TestRookMove) {
  Position position = Position::FromFen("8/2r2n2/8/8/3R4/8/8/5R2 w - - 0 1");

  std::array<std::array<uint8_t, 2>, 2> moves = {{{f1, f7}, {c7, f7}}};

  for (auto [from, to] : moves) {
    Move move = DeduceMove(position, from, to);

    position.Make(move);
  }

  ASSERT_EQ(position.ToFen(), "8/5r2/8/8/3R4/8/8/8 w - - 0 2");
}

TEST(PositionTestSuite, TestQueenMove) {
  Position position = Position::FromFen("8/8/4r3/1n1N2q1/2Q5/8/8/8 w - - 0 1");

  std::array<std::array<uint8_t, 2>, 9> moves = {{
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

  ASSERT_EQ(position.ToFen(), "4Q3/8/q7/3r4/8/8/8/8 b - - 5 5");
}

TEST(PositionTestSuite, TestEnPassantCapture) {
  Position position = Position::FromFen(
      "rnbqkbnr/pppp1ppp/8/8/3pP3/8/PPP2PPP/RNBQKBNR w KQkq - 0 1");

  Move white = DeduceMove(position, c2, c4);

  position.Make(white);

  ASSERT_EQ(position.ToFen(),
            "rnbqkbnr/pppp1ppp/8/8/2PpP3/8/PP3PPP/RNBQKBNR b KQkq c3 0 1");

  Move black = DeduceMove(position, d4, c3);

  position.Make(black);

  ASSERT_EQ(position.ToFen(),
            "rnbqkbnr/pppp1ppp/8/8/4P3/2p5/PP3PPP/RNBQKBNR w KQkq - 0 2");

  position.Undo(black);

  ASSERT_EQ(position.ToFen(),
            "rnbqkbnr/pppp1ppp/8/8/2PpP3/8/PP3PPP/RNBQKBNR b KQkq c3 0 1");

  position.Undo(white);

  ASSERT_EQ(position.ToFen(),
            "rnbqkbnr/pppp1ppp/8/8/3pP3/8/PPP2PPP/RNBQKBNR w KQkq - 0 1");
}

TEST(PositionTestSuite, TestUpdateCastlingRights) {
  Position position = Position::FromFen(
      "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -");

  Move castle_right = DeduceMove(position, e1, g1);

  position.Make(castle_right);

  ASSERT_EQ(
      position.ToFen(),
      "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R4RK1 b kq - 1 1");

  position.Undo(castle_right);

  ASSERT_EQ(
      position.ToFen(),
      "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1");

  Move castle_left = DeduceMove(position, e1, c1);

  position.Make(castle_left);

  ASSERT_EQ(
      position.ToFen(),
      "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/2KR3R b kq - 1 1");

  position.Undo(castle_left);

  ASSERT_EQ(
      position.ToFen(),
      "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1");
}
