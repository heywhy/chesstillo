#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <functional>
#include <iostream>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

#include <engine/board.hpp>
#include <engine/constants.hpp>
#include <engine/move.hpp>
#include <engine/move_gen.hpp>
#include <engine/position.hpp>
#include <engine/scheduler.hpp>
#include <engine/types.hpp>
#include <engine/utility.hpp>

using namespace engine;

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

Stat Perft(Position &position, int depth, bool divide) {
  if (depth == 0) {
    return {static_cast<std::uint64_t>(1)};
  }

  Stat stat;
  MoveList move_list = GenerateMoves(position);

  for (Move &move : move_list) {
    position.Make(move);

    Stat result = Perft(position, depth - 1, false);

    stat += result;

    if (depth - 1 == 0) {
      Bitboard check_mask = CheckMask(position);
      stat.checks += check_mask != kUniverse;
      stat.captures += move.Is(move::CAPTURE) + move.Is(move::EN_PASSANT);
      stat.en_passants += move.Is(move::EN_PASSANT);
      stat.checkmates += move.Is(move::CHECKMATE);
      // stat.discovery_checks += move.Is(DISCOVERY);
      stat.double_checks += check_mask == kEmpty;
      stat.promotions += move.Is(move::PROMOTION);
      stat.castles +=
          move.Is(move::CASTLE_QUEEN_SIDE) || move.Is(move::CASTLE_KING_SIDE);
    }

    position.Undo(move);

    if (divide) {
      char s[6];

      if (ToString(s, move)) {
        stat.map.insert({s, result.nodes});
      }
    }
  }

  return stat;
}

Stat BulkPerft(Position &position, int depth, bool divide) {
  Stat stat;
  MoveList move_list = GenerateMoves(position);

  if (depth == 1) {
    stat.nodes = move_list.size();
    return stat;
  }

  for (Move &move : move_list) {
    position.Make(move);

    Stat result = BulkPerft(position, depth - 1, false);

    stat += result;

    position.Undo(move);

    if (divide) {
      char s[6];

      if (ToString(s, move)) {
        stat.map.insert({s, result.nodes});
      }
    }
  }

  return stat;
}

Stat ThreadedPerft(Scheduler *scheduler, Position &position, int depth,
                   bool divide) {
  Stat stat;
  std::mutex m;
  std::vector<Status *> jobs;
  MoveList move_list = GenerateMoves(position);

  if (depth == 1) {
    stat.nodes = move_list.size();
    return stat;
  }

  auto callback = [](Position position, int depth, bool divide, Move *move,
                     Stat *stat, std::mutex *m) {
    Stat result = BulkPerft(position, depth, false);

    std::lock_guard<std::mutex> lock_guard(*m);

    *stat += result;

    if (divide) {
      char s[6];

      if (ToString(s, *move)) {
        (*stat).map.insert({s, result.nodes});
      }
    }
  };

  for (Move &move : move_list) {
    position.Make(move);

    Status *status = scheduler->Dispatch([&, position]() {
      callback(position, depth - 1, divide, &move, &stat, &m);
    });

    jobs.push_back(status);

    position.Undo(move);
  }

  for (Status *s : jobs) {
    s->Wait();
    delete s;
  }

  return stat;
}

using PerftFun = std::function<Stat(Position &, int, bool)>;

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

  PerftFun perft = BulkPerft;
  Position position = Position::FromFen(kStartPos);

  // TODO: allow workers count to be configurable
  Scheduler scheduler;

  scheduler.Init();

  // ApplyFen(position, "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R
  // w KQkq -");

  perft = [&scheduler](Position &position, int depth, bool divide) {
    return ThreadedPerft(&scheduler, position, depth, divide);
  };

  Run(position, perft, 8 - 2, true);

  return 0;
}
