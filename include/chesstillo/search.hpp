#ifndef SEARCH_HPP
#define SEARCH_HPP

#include <condition_variable>
#include <cstddef>
#include <cstdlib>
#include <mutex>
#include <queue>
#include <set>
#include <vector>

#include "position.hpp"
#include "threads.hpp"
#include "transposition.hpp"
#include "types.hpp"

#define MAX_DEPTH 6 // 125
#define MAX_STACK_MOVES 2000000

class Task;
class TaskStack;

enum Stop { RUNNING, STOP_PARALLEL, STOP_END };

class Search {
public:
  TaskStack *tasks;
  bool allow_node_splitting;
  Stop stop;
  int height;
  int depth;

  Search();
  Search(int tasks);
  ~Search();

  void Clone(Search *);
  void SetState(Stop state);
  void StopAll(Stop state);

private:
  Task *task_;
  Search *master_;
  Search *parent_;

  int nodes_;
  std::vector<Search *> children_;

  SpinLock spin_;

  friend class Node;
  friend class Task;
};

struct Stack {
  int depth;
  int cmove;
  size_t nps;
  Bitboard check_mask;
  Move *moves;

  Stack()
      : depth(0), cmove(0), nps(0), check_mask(kEmpty),
        moves(static_cast<Move *>(std::calloc(MAX_STACK_MOVES, sizeof(Move)))) {
  }

  ~Stack() { std::free(moves); }
};

class SearchManager;

struct SplitPoint {
  Position &position;
  int depth;
  int alpha;
  int beta;
  NodeType node;
  SearchManager *search;
  std::queue<Move> moves;
  std::set<Slave *> active_slaves;
  bool stop_search = false;
  std::mutex mutex;
  std::condition_variable cv;

  void WaitForSlaves();
  void Search(Slave *slave);
  void Finished(Slave *slave);
  void Tackling(Slave *slave);
  void Copy(Moves::iterator begin, Moves::iterator end);
};

class SearchManager {
public:
  SearchManager(TT *tt) : tt_(tt) {}

  Move FindBestMove(Position &position);
  Move TFindBestMove(Position &position);

private:
  TT *tt_;
  SlaveManager slave_manager_;

  friend struct SplitPoint;

  void OrderMoves(Position &position, Moves &moves, int depth);
  int Quiesce(Position &position, int alpha, int beta);

  template <enum NodeType T>
  int Search(Position &position, int alpha, int beta, Stack &stack);

  template <enum NodeType T>
  int TSearch(Position &position, int alpha, int beta, Stack &stack);

  void SplitPointSearch(SplitPoint *split_point, Slave *slave);
  SplitPoint Split(Position &, int alpha, int beta, Moves &moves);
};
#endif
