#ifndef SEARCH_HPP
#define SEARCH_HPP

#include <condition_variable>
#include <cstddef>
#include <mutex>
#include <thread>
#include <vector>

#include <engine/position.hpp>
#include <engine/transposition.hpp>
#include <engine/types.hpp>

// #define MAX_DEPTH 10  // 125
#define MAX_DEPTH 8
#define MAX_THREADS 256

namespace engine {
class Search;

namespace search {

class Node {
 public:
  Search *search;
  int alpha;
  int beta;
  int depth;

  Node *parent;

  int score;
  NodeType type;
  Move best_move;

  Node(Search *search, int alpha, int beta, int depth);
  Node(Search *search, int alpha, int beta, int depth, Node *parent);

  Node(const Node &) = delete;
};

class WorkerRegistry;

class Worker {
 public:
  WorkerRegistry *registry;

  Worker();
  ~Worker();

  void Search();
  void Assign(Node *node);

 private:
  bool loop_;
  Node *node_;
  std::size_t nodes_;

  class Search *search_;

  std::mutex mutex_;
  std::thread thread_;
  std::condition_variable cv_;

  void Loop();
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

  Search(search::WorkerRegistry *workers);
  Search(const Search &) = delete;

  void Run();
  void Clone(Search *search);

 private:
  int depth_;
  search::WorkerRegistry *workers_;

  template <enum NodeType T>
  int search(int alpha, int beta, int depth, search::Node *parent);

  int Quiesce(int alpha, int beta);
};
}  // namespace engine

#endif
