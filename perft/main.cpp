#include <cstdint>
#include <cstdio>

#include <chesstillo/board.hpp>
#include <chesstillo/fen.hpp>
#include <chesstillo/move.hpp>
#include <chesstillo/types.hpp>
#include <vector>

std::uint64_t Perft(Board &board, int depth) {
  if (depth == 0) {
    return static_cast<std::uint64_t>(1);
  }

  std::uint64_t nodes = 0;
  std::vector<Move> moves = GenerateMoves(board);

  for (Move &move : moves) {
    board.ApplyMove(move);
    nodes += Perft(board, depth - 1);
    board.UndoMove(move);
  }

  return nodes;
}

void Run(int depth) {
  Board board;

  ApplyFen(board, START_FEN);

  std::printf("nodes count for depth %d is %llu\n", depth, Perft(board, depth));
}

int main() {
  std::printf("============================================\n");
  std::printf("starting perf tests\n");
  std::printf("============================================\n");

  Run(0);
  Run(1);
  Run(2);
  Run(3);
  Run(4);

  return 0;
}
