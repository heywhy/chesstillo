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
  std::size_t en_passants = 0;
  std::size_t discovery_checks = 0;
  std::size_t double_checks = 0;
  std::size_t promotions = 0;
  std::map<std::string, int> map{};

  void operator+=(Stat &stat) {
    nodes += stat.nodes;
    checks += stat.checks;
    captures += stat.captures;
    checkmates += stat.checkmates;
    en_passants += stat.en_passants;
    discovery_checks += stat.discovery_checks;
    double_checks += stat.double_checks;
    promotions += stat.promotions;
  }
};

Stat Perft(Board &board, int depth, bool divide) {
  if (depth == 0) {
    return {static_cast<std::uint64_t>(1)};
  }

  Stat stat;
  std::vector<Move> moves = GenerateMoves(board);

  char s[5];
  MoveToString(board.GetMoves().front(), s);

  // if (std::strcmp(s, "c5+") == 0) {
  //   for (Move &a : moves) {
  //     MoveToString(a, s);
  //     std::cout << a.piece << std::endl;
  //     std::cout << s << std::endl;
  //   }
  //
  //   std::cout << "caught me =" << moves.size() << std::endl;
  //   std::cout << "caught me =" << board.GetTurn() << std::endl;
  // }

  for (Move &move : moves) {
    board.ApplyMove(move);

    Stat result = Perft(board, depth - 1, false);

    stat += result;

    if (depth - 1 == 0) {
      stat.checks += move.Is(CHECK);
      stat.captures += move.Is(CAPTURE);
      stat.checkmates += move.Is(CHECKMATE);
      stat.en_passants += move.Is(EN_PASSANT);
      stat.discovery_checks += move.Is(DISCOVERY);
      stat.double_checks += move.Is(DOUBLE);
      stat.promotions += move.Is(PROMOTION);
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

void Run(Board &board, int depth, bool divide) {
  Stat stat = Perft(board, depth, divide);

  for (auto [k, v] : stat.map) {
    std::cout << k << " " << v << std::endl;
  }

  std::printf(
      "stats for depth %d is nodes=%lu, captures=%lu, e.p=%lu, promotions=%lu, "
      "checks=%lu, discovery checks=%lu, double checks=%lu, checkmates=%lu\n",
      depth, stat.nodes, stat.captures, stat.en_passants, stat.promotions,
      stat.checks, stat.discovery_checks, stat.double_checks, stat.checkmates);

  if (divide) {
    std::cout << std::endl;
  }
}

int main() {
  std::printf("============================================\n");
  std::printf("starting perf tests\n");
  std::printf("============================================\n");

  for (int i = 0; i < 6; i++) {
    Run(i, false);
  }

  return 0;
}
