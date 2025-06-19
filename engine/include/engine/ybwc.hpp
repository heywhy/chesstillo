#ifndef YBWC_HPP
#define YBWC_HPP

#include <condition_variable>
#include <mutex>
#include <thread>
#include <vector>

#include "move.hpp"
#include "search.hpp"
#include "settings.hpp"
#include "types.hpp"

class Node;

class Task {
 public:
  Task();
  ~Task();

 private:
  bool run_;
  bool stop_;
  bool helping_;

  Move *move_;
  Node *node_;
  Search *search_;
  TaskStack *container_;

  std::mutex mutex_;
  std::thread thread_;
  std::condition_variable cv_;

  void Loop();
  void Search();

  friend class Node;
  friend class Search;
  friend class TaskStack;
  friend bool GetHelper(Node *master, Node *node, Move *move);
};

class Node {
 public:
  int alpha;
  int beta;
  int depth;
  int moves_done;
  int moves_todo;
  bool pv_node;

  Node(Search *search, int alpha, int beta, int depth);
  Node(Search *search, int alpha, int beta, int depth, Node *parent);
  ~Node();

  bool Split(Move &move);
  void WaitSlaves();
  void Update(Move &move);

 private:
  Node *parent_;
  Search *search_;
  Task *help_;

  int height_;
  int best_score_;

  Move *move_;
  Move best_move_;
  std::vector<Search *> slaves_;

  bool helping_;
  bool waiting_;
  bool stop_point_;

  std::mutex mutex_;
  std::condition_variable cv_;

  friend class Task;
  friend class Search;
  friend bool GetHelper(Node *master, Node *node, Move *move);

  Move *NextMove();
  Move *NextMoveLockless();
  Move *FirstMove(MoveList &move_list);
};

class TaskStack;

class TaskStack {
 public:
  TaskStack(int size);

  Task *GetIdleTask();
  void PutIdleTask(Task *task);

 private:
  int size_;
  int idle_;

  std::mutex mutex_;
  std::vector<Task> tasks_;
  std::vector<Task *> stack_;
};

#endif
