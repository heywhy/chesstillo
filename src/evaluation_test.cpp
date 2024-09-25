#include <chesstillo/evaluation.hpp>
#include <chesstillo/fen.hpp>
#include <chesstillo/position.hpp>
#include <chesstillo/types.hpp>

#include <gtest/gtest.h>

class EvaluationTestSuite : public testing::Test {
protected:
  Position position;

  void SetUp() override { ApplyFen(position, START_FEN); }
  void TearDown() override { position.Reset(); }
};

TEST_F(EvaluationTestSuite, EvalIsolatedPawns) {}

TEST_F(EvaluationTestSuite, EvalBackwardPawns) {
  ApplyFen(position, "8/8/P1P5/8/1P6/8/8/8 w - - 0 1");

  EvalState state = EvalState::For(position);
  Bitboard white_pawns = state.white_pieces[PAWN];
  Bitboard black_pawns = state.black_pieces[PAWN];
  auto result = BackwardPawns<WHITE>(white_pawns, black_pawns);

  ASSERT_EQ(std::get<0>(result), 1);

  ApplyFen(position, "8/8/2P5/P7/1P6/8/8/8 w - - 0 1");

  state = EvalState::For(position);
  white_pawns = state.white_pieces[PAWN];
  black_pawns = state.black_pieces[PAWN];
  result = BackwardPawns<WHITE>(white_pawns, black_pawns);

  ASSERT_EQ(std::get<0>(result), 0);

  ApplyFen(position, "8/8/2P5/Pp6/1P6/8/8/8 w - - 0 1");

  state = EvalState::For(position);
  white_pawns = state.white_pieces[PAWN];
  black_pawns = state.black_pieces[PAWN];
  result = BackwardPawns<WHITE>(white_pawns, black_pawns);

  ASSERT_EQ(std::get<0>(result), 1);

  ApplyFen(position, "8/8/8/P1p5/1P6/8/8/8 w - - 0 1");

  state = EvalState::For(position);
  white_pawns = state.white_pieces[PAWN];
  black_pawns = state.black_pieces[PAWN];
  result = BackwardPawns<WHITE>(white_pawns, black_pawns);

  ASSERT_EQ(std::get<0>(result), 1);

  ApplyFen(position, "8/8/2p5/P7/1P6/8/8/8 w - - 0 1");

  state = EvalState::For(position);
  white_pawns = state.white_pieces[PAWN];
  black_pawns = state.black_pieces[PAWN];
  result = BackwardPawns<WHITE>(white_pawns, black_pawns);

  ASSERT_EQ(std::get<0>(result), 1);

  ApplyFen(position, "8/8/2p5/PP6/8/8/8/8 w - - 0 1");

  state = EvalState::For(position);
  white_pawns = state.white_pieces[PAWN];
  black_pawns = state.black_pieces[PAWN];
  result = BackwardPawns<WHITE>(white_pawns, black_pawns);

  ASSERT_EQ(std::get<0>(result), 0);

  ApplyFen(position, "8/8/8/8/P7/8/1P6/8 w - - 0 1");

  state = EvalState::For(position);
  white_pawns = state.white_pieces[PAWN];
  black_pawns = state.black_pieces[PAWN];
  result = BackwardPawns<WHITE>(white_pawns, black_pawns);

  ASSERT_EQ(std::get<0>(result), 0);

  ApplyFen(position, "8/8/8/8/PP6/8/1P6/8 w - - 0 1");

  state = EvalState::For(position);
  white_pawns = state.white_pieces[PAWN];
  black_pawns = state.black_pieces[PAWN];
  result = BackwardPawns<WHITE>(white_pawns, black_pawns);

  ASSERT_EQ(std::get<0>(result), 1);

  ApplyFen(position, "8/8/8/2p5/P7/8/1P6/8 w - - 0 1");

  state = EvalState::For(position);
  white_pawns = state.white_pieces[PAWN];
  black_pawns = state.black_pieces[PAWN];
  result = BackwardPawns<WHITE>(white_pawns, black_pawns);

  ASSERT_EQ(std::get<0>(result), 1);
}

TEST_F(EvaluationTestSuite, ScoreStartingPosition) {
  ASSERT_EQ(Evaluate(position), 20);
}

TEST_F(EvaluationTestSuite, RandomPositionOne) {
  ApplyFen(position,
           "1r3rk1/3bb1pp/2p1p3/1p2Pp1Q/2pP4/1P4P1/q3NPBP/2RR2K1 w - - 0 1");

  ASSERT_EQ(Evaluate(position), -136);
}

TEST_F(EvaluationTestSuite, RandomPositionTwo) {
  ApplyFen(position,
           "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1");

  ASSERT_EQ(Evaluate(position), 184);
}
