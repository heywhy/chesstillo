#include <chesstillo/board.hpp>
#include <chesstillo/fen.hpp>
#include <gtest/gtest.h>

class BoardTestSuite : public ::testing::Test {
protected:
  Board board;

  void SetUp() override { ApplyFen(board, START_FEN); }

  void TearDown() override { board.Reset(); }
};

TEST_F(BoardTestSuite, ApplyPawnMove) {
  Bitboard empty_squares = board.EmptySquares();

  Bitboard e2 = BitboardForSquare('e', 2);
  Bitboard e4 = BitboardForSquare('e', 4);

  Move move = {e2, e4, WHITE, PAWN};

  board.ApplyMove(move);

  ASSERT_NE(board.EmptySquares(), empty_squares);
  ASSERT_TRUE(board.Position(PAWN, WHITE) & e4);
}

TEST_F(BoardTestSuite, FailToApplyInvalidPawnMove) {
  Bitboard e2 = BitboardForSquare('e', 2);
  Bitboard d4 = BitboardForSquare('d', 4);

  Move move = {e2, d4, WHITE, PAWN};

  board.ApplyMove(move);

  ASSERT_EQ(PositionToFen(board), START_FEN);
}

TEST_F(BoardTestSuite, ApplyPawnCaptureMove) {
  Move moves[] = {
      Move(BitboardForSquare('e', 2), BitboardForSquare('e', 4), WHITE, PAWN),
      Move(BitboardForSquare('d', 7), BitboardForSquare('d', 5), BLACK, PAWN),
      Move(BitboardForSquare('e', 4), BitboardForSquare('d', 5), WHITE, PAWN),
      Move(BitboardForSquare('c', 7), BitboardForSquare('c', 6), BLACK, PAWN),
      Move(BitboardForSquare('d', 2), BitboardForSquare('d', 4), WHITE, PAWN),
      Move(BitboardForSquare('c', 6), BitboardForSquare('d', 5), BLACK, PAWN),
  };

  for (Move &move : moves) {
    board.ApplyMove(move);
  }

  ASSERT_EQ(PositionToFen(board),
            "rnbqkbnr/pp2pppp/8/3p4/3P4/8/PPP2PPP/RNBQKBNR w KQkq - 0 4");
}

TEST_F(BoardTestSuite, FailToCaptureOwnPawn) {
  Move moves[] = {
      Move(BitboardForSquare('e', 2), BitboardForSquare('e', 4), WHITE, PAWN),
      Move(BitboardForSquare('c', 7), BitboardForSquare('c', 5), BLACK, PAWN),
      Move(BitboardForSquare('d', 2), BitboardForSquare('d', 3), WHITE, PAWN),
      Move(BitboardForSquare('e', 7), BitboardForSquare('e', 6), BLACK, PAWN),
      Move(BitboardForSquare('d', 3), BitboardForSquare('e', 4), WHITE, PAWN),
  };

  for (Move &move : moves) {
    board.ApplyMove(move);
  }

  ASSERT_EQ(PositionToFen(board),
            "rnbqkbnr/pp1p1ppp/4p3/2p5/4P3/3P4/PPP2PPP/RNBQKBNR w KQkq - 0 3");
}

TEST_F(BoardTestSuite, MoveKnight) {
  ApplyFen(board, "8/8/4n3/8/3N4/8/8/8 w - - 0 1");

  board.ApplyMove(
      {BitboardForSquare('d', 4), BitboardForSquare('e', 6), WHITE, KNIGHT});

  ASSERT_EQ(PositionToFen(board), "8/8/4N3/8/8/8/8/8 w - - 0 1");
}

TEST_F(BoardTestSuite, IgnoreInvalidKnightMove) {
  ApplyFen(board, "8/8/8/8/3N4/8/8/8 w - - 0 1");

  board.ApplyMove(
      {BitboardForSquare('d', 4), BitboardForSquare('e', 5), WHITE, KNIGHT});

  ASSERT_EQ(PositionToFen(board), "8/8/8/8/3N4/8/8/8 w - - 0 1");
}

TEST_F(BoardTestSuite, MoveBishop) {
  ApplyFen(board, "8/8/8/3b4/3B4/8/8/8 w - - 0 1");

  Move white(BitboardForSquare('d', 4), BitboardForSquare('b', 6), WHITE,
             BISHOP);
  Move black(BitboardForSquare('d', 5), BitboardForSquare('f', 7), BLACK,
             BISHOP);

  board.ApplyMove(white);
  board.ApplyMove(black);

  ASSERT_EQ(PositionToFen(board), "8/5b2/1B6/8/8/8/8/8 w - - 2 2");
}

TEST_F(BoardTestSuite, IgnoreInvalidBishopMove) {
  char const *fen = "8/8/8/8/3B4/8/8/8 w - - 0 1";

  ApplyFen(board, fen);

  Bitboard piece = BitboardForSquare('d', 4);

  Move moves[] = {
      Move(piece, BitboardForSquare('c', 4), WHITE, BISHOP),
      Move(piece, BitboardForSquare('e', 4), WHITE, BISHOP),
      Move(piece, BitboardForSquare('d', 5), WHITE, BISHOP),
      Move(piece, BitboardForSquare('d', 3), WHITE, BISHOP),
  };

  for (int i = 0; i < 4; i++) {
    board.ApplyMove(moves[i]);
  }

  ASSERT_EQ(PositionToFen(board), fen);
}

TEST_F(BoardTestSuite, IgnoreBishopMoveIfBlocked) {
  ApplyFen(board, "8/8/8/2r1Pn2/3B4/2R5/8/8 w - - 0 1");

  Bitboard piece = BitboardForSquare('d', 4);

  Move moves[] = {
      Move(piece, BitboardForSquare('b', 2), WHITE, BISHOP),
      Move(piece, BitboardForSquare('f', 6), WHITE, BISHOP),
      Move(piece, BitboardForSquare('c', 5), WHITE, BISHOP),
      Move(BitboardForSquare('f', 5), BitboardForSquare('e', 3), BLACK, KNIGHT),
      Move(BitboardForSquare('c', 5), BitboardForSquare('e', 3), WHITE, BISHOP),
  };

  for (int i = 0; i < 5; i++) {
    board.ApplyMove(moves[i]);
  }

  ASSERT_EQ(PositionToFen(board), "8/8/8/4P3/8/2R1B3/8/8 w - - 0 2");
}

TEST_F(BoardTestSuite, MoveRook) {
  ApplyFen(board, "8/2r2n2/8/8/3R4/8/8/5R2 w - - 0 1");

  Move white(BitboardForSquare('f', 1), BitboardForSquare('f', 7), WHITE, ROOK);
  Move black(BitboardForSquare('c', 7), BitboardForSquare('f', 7), BLACK, ROOK);

  board.ApplyMove(white);
  board.ApplyMove(black);

  ASSERT_EQ(PositionToFen(board), "8/5r2/8/8/3R4/8/8/8 w - - 0 2");
}

TEST_F(BoardTestSuite, IgnoreInvalidRookMove) {
  char const *fen = "8/8/8/8/4R3/8/8/8 w - - 0 1";

  ApplyFen(board, fen);

  Bitboard piece = BitboardForSquare('e', 4);

  Move moves[] = {
      Move(piece, BitboardForSquare('d', 3), WHITE, ROOK),
      Move(piece, BitboardForSquare('d', 5), WHITE, ROOK),
      Move(piece, BitboardForSquare('f', 5), WHITE, ROOK),
      Move(piece, BitboardForSquare('f', 3), WHITE, ROOK),
  };

  for (int i = 0; i < 4; i++) {
    board.ApplyMove(moves[i]);
  }

  ASSERT_EQ(PositionToFen(board), fen);
}

TEST_F(BoardTestSuite, IgnoreRookMoveIfBlocked) {
  ApplyFen(board, "8/8/5n2/4P3/1qBR2r1/8/8/8 w - - 0 1");

  Bitboard piece = BitboardForSquare('d', 4);

  Move moves[] = {
      Move(piece, BitboardForSquare('b', 4), WHITE, ROOK),
      Move(piece, BitboardForSquare('g', 4), WHITE, ROOK),
      Move(BitboardForSquare('f', 6), BitboardForSquare('g', 4), BLACK, KNIGHT),
      Move(BitboardForSquare('c', 4), BitboardForSquare('e', 6), WHITE, BISHOP),
  };

  for (int i = 0; i < 5; i++) {
    board.ApplyMove(moves[i]);
  }

  ASSERT_EQ(PositionToFen(board), "8/8/4B3/4P3/1q4n1/8/8/8 b - - 1 2");
}

TEST_F(BoardTestSuite, MoveQueen) {
  ApplyFen(board, "8/8/4r3/1n1N2q1/2Q5/8/8/8 w - - 0 1");

  Bitboard piece = BitboardForSquare('c', 4);

  Move moves[] = {
      Move(piece, BitboardForSquare('e', 6), WHITE, QUEEN),
      Move(piece, BitboardForSquare('b', 5), WHITE, QUEEN),
      Move(BitboardForSquare('e', 6), BitboardForSquare('e', 5), BLACK, ROOK),
      Move(BitboardForSquare('b', 5), BitboardForSquare('f', 1), WHITE, QUEEN),
      Move(BitboardForSquare('e', 5), BitboardForSquare('d', 5), BLACK, ROOK),
      Move(BitboardForSquare('f', 1), BitboardForSquare('e', 2), WHITE, QUEEN),
      Move(BitboardForSquare('g', 5), BitboardForSquare('h', 6), BLACK, QUEEN),
      Move(BitboardForSquare('e', 2), BitboardForSquare('e', 8), WHITE, QUEEN),
      Move(BitboardForSquare('h', 6), BitboardForSquare('a', 6), BLACK, QUEEN),
  };

  for (int i = 0; i < 9; i++) {
    board.ApplyMove(moves[i]);
  }

  ASSERT_EQ(PositionToFen(board), "4Q3/8/q7/3r4/8/8/8/8 w - - 4 5");
}

TEST_F(BoardTestSuite, IgnoreQueenMoveIfBlocked) {
  board.ApplyMove(
      {BitboardForSquare('d', 1), BitboardForSquare('g', 4), WHITE, QUEEN});

  ASSERT_EQ(PositionToFen(board), START_FEN);
}

TEST_F(BoardTestSuite, MoveKingAround) {
  ApplyFen(board, "8/8/8/8/3K4/8/8/8 w - - 0 1");

  Bitboard piece = BitboardForSquare('d', 4);

  board.ApplyMove({piece, BitboardForSquare('f', 6), WHITE, KING});
  board.ApplyMove({piece, BitboardForSquare('e', 5), WHITE, KING});

  ASSERT_EQ(PositionToFen(board), "8/8/8/4K3/8/8/8/8 w - - 1 1");
}

TEST_F(BoardTestSuite, PiecesCantCaptureOwnsPieces) {
  board.ApplyMove(
      {BitboardForSquare('e', 1), BitboardForSquare('f', 2), WHITE, KING});
  board.ApplyMove(
      {BitboardForSquare('e', 1), BitboardForSquare('d', 2), WHITE, KING});
  board.ApplyMove(
      {BitboardForSquare('e', 1), BitboardForSquare('d', 2), WHITE, KING});

  board.ApplyMove(
      {BitboardForSquare('d', 1), BitboardForSquare('c', 2), WHITE, QUEEN});
  board.ApplyMove(
      {BitboardForSquare('d', 1), BitboardForSquare('d', 2), WHITE, QUEEN});
  board.ApplyMove(
      {BitboardForSquare('d', 1), BitboardForSquare('e', 2), WHITE, QUEEN});

  board.ApplyMove(
      {BitboardForSquare('b', 1), BitboardForSquare('d', 2), WHITE, KNIGHT});
  board.ApplyMove(
      {BitboardForSquare('g', 1), BitboardForSquare('e', 2), WHITE, KNIGHT});

  ASSERT_EQ(PositionToFen(board), START_FEN);
}
