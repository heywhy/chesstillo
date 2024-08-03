#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <iostream>
#include <map>
#include <string>
#include <vector>

#include <chesstillo/board.hpp>
#include <chesstillo/fen.hpp>
#include <chesstillo/move_gen.hpp>
#include <chesstillo/types.hpp>

struct Stat {
  std::size_t nodes = 0;
  std::size_t checks = 0;
  std::size_t captures = 0;
  std::size_t checkmates = 0;
  std::map<std::string, int> map{};

  void operator+=(Stat &stat) {
    nodes += stat.nodes;
    checks += stat.checks;
    captures += stat.captures;
    checkmates += stat.checkmates;
  }
};

Stat Perft(Board &board, int depth, bool divide) {
  if (depth == 0) {
    return {static_cast<std::uint64_t>(1)};
  }

  Stat stat;
  std::vector<Move> moves = GenerateMoves(board);

  for (Move &move : moves) {
    board.ApplyMove(move);

    Stat result = Perft(board, depth - 1, divide);

    stat += result;

    if (depth - 1 == 0) {
      Move &last_move = board.GetMoves().front();

      stat.checks += last_move.Is(CHECK);
      stat.captures += last_move.Is(CAPTURE);
      stat.checkmates += last_move.Is(CHECKMATE);
    }

    board.UndoMove(move);

    if (divide) {
      char s[5];

      if (MoveToString(move, s)) {
        stat.map.insert({s, result.nodes});
      }
    }
  }

  return stat;
}

void Run(int depth, bool divide) {
  Board board;

  ApplyFen(board, START_FEN);

  Stat stat = Perft(board, depth, divide);

  for (auto [k, v] : stat.map) {
    std::cout << k << " " << v << std::endl;
  }

  std::printf("stats for depth %d is nodes=%lu, captures=%lu, checks=%lu, "
              "checkmates=%lu\n",
              depth, stat.nodes, stat.captures, stat.checks, stat.checkmates);

  if (divide) {
    std::cout << std::endl;
  }
}

int main() {
  std::printf("============================================\n");
  std::printf("starting perf tests\n");
  std::printf("============================================\n");

  for (int i = 0; i < 5; i++) {
    Run(i, false);
  }

  return 0;
}
