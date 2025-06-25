#ifndef SEARCH_HPP
#define SEARCH_HPP

#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <vector>

#include "constants.hpp"
#include "move.hpp"
#include "position.hpp"
#include "threads.hpp"
#include "types.hpp"

#define MAX_STACK_MOVES 2000000

namespace engine {

class Node;
class Task;
class TaskStack;

enum Stop { RUNNING, STOP_PARALLEL, STOP_END };

class Result {
 private:
  Line pv_;
  int depth_;
  std::uint64_t nodes_;
  SpinLock spin_;

  int best_score_;
  Move *best_move_;

  int moves_;
  int moves_left_;

  friend class Node;
  friend class Task;
  friend class Search;
};

class Search {
 public:
  TaskStack *tasks;
  Position *position;

  bool allow_node_splitting;
  Stop stop;
  int depth;

  Search();
  Search(int tasks);
  ~Search();

  void Run();
  void Clone(Search *);
  void SetState(Stop state);
  void StopAll(Stop state);
  int Midgame(Move &move, int alpha, int depth, Node *node);

 private:
  Task *task_;
  Search *master_;
  Search *parent_;

  int nodes_;
  int height_;
  Result *result_;
  NodeType node_type_[MAX_DEPTH];
  std::vector<Search *> children_;

  SpinLock spin_;

  void UpdateMidgame(Move &move);
  void RestoreMidgame(Move &move);
  void Iterative(int alpha, int depth);
  int Aspiration(int alpha, int beta, int depth, int score);

  int PVS(int alpha, int beta, int depth, Node *parent);

  int PVS_Root(int alpha, int beta, int depth);
  int NWS_Midgame(int alpha, int depth, Node *node);
  int PVS_Midgame(int alpha, int beta, int depth, Node *node);

  void RecordBestMove(const Move &move, const int alpha, const int beta,
                      const int depth);

  static int Quiesce(Position &position, int alpha, int beta);

  friend class Node;
  friend class Task;
};

struct Stack {
  int depth;
  int cmove;
  std::size_t nps;
  Bitboard check_mask;
  Move *moves;

  Stack()
      : depth(0),
        cmove(0),
        nps(0),
        check_mask(kEmpty),
        moves(static_cast<Move *>(std::calloc(MAX_STACK_MOVES, sizeof(Move)))) {
  }

  ~Stack() { std::free(moves); }
};

}  // namespace engine

#endif
