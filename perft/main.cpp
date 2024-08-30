#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <functional>
#include <iostream>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

#include <chesstillo/board.hpp>
#include <chesstillo/constants.hpp>
#include <chesstillo/fen.hpp>
#include <chesstillo/move_gen.hpp>
#include <chesstillo/position.hpp>
#include <chesstillo/types.hpp>
#include <chesstillo/utility.hpp>

struct Stat {
  std::size_t nodes = 0;
  std::size_t checks = 0;
  std::size_t captures = 0;
  std::size_t checkmates = 0;
  std::size_t en_passants = 0;
  std::size_t discovery_checks = 0;
  std::size_t double_checks = 0;
  std::size_t promotions = 0;
  std::size_t castles = 0;
  std::unordered_map<std::string, std::size_t> map{};

  Stat &operator+=(const Stat &stat) {
    nodes += stat.nodes;
    checks += stat.checks;
    captures += stat.captures;
    checkmates += stat.checkmates;
    en_passants += stat.en_passants;
    discovery_checks += stat.discovery_checks;
    double_checks += stat.double_checks;
    promotions += stat.promotions;
    castles += stat.castles;

    return *this;
  }
};

bool ToString(Move const &move, char *buf) {
  Coord to;
  Coord from;

  if (!CoordForSquare(&from, move.from) || !CoordForSquare(&to, move.to)) {
    return false;
  }

  buf[0] = from.file;
  buf[1] = 48 + from.rank;

  buf[2] = to.file;
  buf[3] = 48 + to.rank;

  if (move.Is(PROMOTION)) {
    char piece;
    PieceToChar(move.promoted, &piece);

    buf[4] = piece;
    buf[5] = '\0';
  } else {
    buf[4] = '\0';
  }

  return true;
}

Stat Perft(Position &position, int depth, bool divide) {
  if (depth == 0) {
    return {static_cast<std::uint64_t>(1)};
  }

  Stat stat;
  std::vector<Move> moves = GenerateMoves(position);

  for (Move &move : moves) {
    position.Make(move);

    Stat result = Perft(position, depth - 1, false);

    stat += result;

    if (depth - 1 == 0) {
      Bitboard check_mask = CheckMask(position);
      stat.checks += check_mask != kUniverse;
      stat.captures += move.Is(CAPTURE) + move.Is(EN_PASSANT);
      stat.en_passants += move.Is(EN_PASSANT);
      stat.checkmates += move.Is(CHECKMATE);
      // stat.discovery_checks += move.Is(DISCOVERY);
      stat.double_checks += check_mask == kEmpty;
      stat.promotions += move.Is(PROMOTION);
      stat.castles += move.Is(CASTLE_RIGHT) || move.Is(CASTLE_LEFT);
    }

    position.Undo(move);

    if (divide) {
      char s[6];

      if (ToString(move, s)) {
        stat.map.insert({s, result.nodes});
      }
    }
  }

  return stat;
}

Stat BulkPerft(Position &position, int depth, bool divide) {
  Stat stat;
  std::vector<Move> moves = GenerateMoves(position);

  if (depth == 1) {
    stat.nodes = moves.size();
    return stat;
  }

  for (Move &move : moves) {
    position.Make(move);

    Stat result = BulkPerft(position, depth - 1, false);

    stat += result;

    position.Undo(move);

    if (divide) {
      char s[6];

      if (ToString(move, s)) {
        stat.map.insert({s, result.nodes});
      }
    }
  }

  return stat;
}

Stat ThreadedPerft(Position &position, int depth, bool divide) {
  Stat stat;
  std::mutex m;
  std::vector<std::thread> threads;

  std::vector<Move> moves = GenerateMoves(position);

  if (depth == 1) {
    stat.nodes = moves.size();
    return stat;
  }

  auto callback = [](Position position, int depth, bool divide, Move *move,
                     Stat *stat, std::mutex *m) {
    Stat result = BulkPerft(position, depth, false);

    std::lock_guard<std::mutex> lock_guard(*m);

    *stat += result;

    if (divide) {
      char s[6];

      if (ToString(*move, s)) {
        (*stat).map.insert({s, result.nodes});
      }
    }
  };

  for (Move &move : moves) {
    position.Make(move);

    threads.emplace_back(callback, position, depth - 1, divide, &move, &stat,
                         &m);

    position.Undo(move);
  }

  for (std::thread &thread : threads) {
    thread.join();
  }

  return stat;
}

typedef std::function<Stat(Position &, int, bool)> PerftFun;

void Run(Position &position, PerftFun perft, int depth, bool divide) {
  Stat stat = perft(position, depth, divide);

  for (auto [k, v] : stat.map) {
    std::cout << k << ": " << v << std::endl;
  }

  std::printf(
      "stats for depth %d is nodes=%lu, captures=%lu, e.p=%lu, castles=%lu, "
      "promotions=%lu, checks=%lu, discovery checks=%lu, double checks=%lu, "
      "checkmates=%lu\n",
      depth, stat.nodes, stat.captures, stat.en_passants, stat.castles,
      stat.promotions, stat.checks, stat.discovery_checks, stat.double_checks,
      stat.checkmates);

  if (divide) {
    std::cout << std::endl;
  }
}

int main() {
  std::printf("============================================\n");
  std::printf("starting perf tests\n");
  std::printf("============================================\n");

  Position position;
  PerftFun perft = BulkPerft;

  ApplyFen(position, START_FEN);

  perft = ThreadedPerft;
  Run(position, perft, 8 - 0, true);

  return 0;
}
