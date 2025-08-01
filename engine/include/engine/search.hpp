#ifndef ENGINE_SEARCH_HPP
#define ENGINE_SEARCH_HPP

#include <condition_variable>
#include <cstddef>
#include <mutex>
#include <thread>
#include <vector>

#include <engine/config.hpp>
#include <engine/move.hpp>
#include <engine/position.hpp>
#include <engine/threads.hpp>
#include <engine/transposition.hpp>
#include <engine/types.hpp>

namespace engine {
class Search;

namespace search {

enum class State { RUNNING, STOP_PARALLEL, END };

class Worker;

class Node {
 public:
  Search *search;
  int alpha;
  int beta;
  int depth;

  Node *parent;

  int best_score;
  NodeType type;
  Move best_move;

  Node(Search *search, int alpha, int beta, int depth);
  Node(Search *search, int alpha, int beta, int depth, Node *parent);

  Node(const Node &) = delete;

  Move *NextMove();
  Move *NextMoveLockless();
  Move *FirstMove(const MoveList &move_list);
  void Update(const Move &move);

  void WaitSlaves();
  void AddSlave(Search *search);
  void RemoveSlave(Search *search);

  bool Split(const Move &move);

 private:
  Worker *help_;
  std::vector<Search *> slaves_;

  bool helping_;
  bool waiting_;
  bool stop_point_;

  const Move *move_;

  std::size_t moves_done_;
  std::size_t moves_todo_;

  std::mutex mutex_;
  std::condition_variable cv_;

  friend class Worker;

  static bool GetHelper(Node *master, Node *node, const Move &move);
};

class WorkerRegistry {
 public:
  WorkerRegistry(std::size_t count);

  Worker *GetIdleWorker();
  std::size_t IdleWorkers();
  void PutIdleWorker(Worker *worker);

 private:
  std::size_t idle_;

  std::vector<Worker> workers_;
  std::vector<Worker *> stack_;

  std::mutex mutex_;
};

}  // namespace search

class Search {
 public:
  TT *tt;
  Position *position;
  search::State state;
  bool allow_node_splitting;
  search::WorkerRegistry *workers;

  Search(search::WorkerRegistry *workers);
  Search(const Search &) = delete;

  void Run();
  void Clone(Search *search);

  void StopAll(search::State new_state);
  void SetState(search::State new_state);

  void Detach();
  void AddChild(Search *child);
  void RemoveChild(Search *child);

  inline bool Continue() { return state == search::State::RUNNING; }

 private:
  int depth_;
  int height_;
  SpinLock spin_;
  std::vector<Search *> children_;

  Search *parent_;
  Search *master_;

  friend class search::Worker;

  template <enum NodeType T>
  int search(int alpha, int beta, int depth, search::Node *parent);

  template <enum NodeType T = NodeType::CUT>
  int NW_Search(int alpha, int depth, search::Node *parent);

  int Quiesce(int alpha, int beta);
  static bool OrderMove(const Move &a, const Move &b);
  static void OrderMoves(MoveList &move_list);
};

namespace search {
class Worker {
 public:
  WorkerRegistry *registry;

  Worker(bool loop = true);
  ~Worker();

  void Search();
  void Assign(Node *node, Move *move);

 private:
  bool loop_;
  Node *node_;
  Move *move_;
  std::size_t nodes_;

  Position position_;
  class Search search_;

  std::mutex mutex_;
  std::thread thread_;
  std::condition_variable cv_;

  void Loop();
};
}  // namespace search
}  // namespace engine

#endif
