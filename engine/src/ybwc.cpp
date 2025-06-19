#include <cassert>
#include <cstdlib>
#include <engine/move.hpp>
#include <engine/search.hpp>
#include <engine/settings.hpp>
#include <engine/types.hpp>
#include <engine/ybwc.hpp>
#include <mutex>
#include <thread>
#include <vector>

Node::Node(Search *search, int alpha, int beta, int depth)
    : Node(search, alpha, beta, depth, nullptr) {}

Node::Node(Search *search, int alpha, int beta, int depth, Node *parent)
    : alpha(alpha),
      beta(beta),
      depth(depth),
      moves_done(0),
      moves_todo(0),
      pv_node(false),
      parent_(parent),
      search_(search),
      help_(nullptr),
      height_(search_->height_),
      best_score_(-SCORE_INF),
      best_move_(Move::NONE),
      helping_(false),
      waiting_(false) {
  assert(MIN_SCORE <= alpha && alpha <= MAX_SCORE);
  assert(MIN_SCORE <= beta && beta <= MAX_SCORE);
  assert(alpha < beta);
}

Node::~Node() { delete help_; }

bool Node::Split(Move &move) {
  if (search_->allow_node_splitting && depth >= SPLIT_MIN_DEPTH && moves_done &&
      slaves_.size() < SPLIT_MAX_SLAVE && moves_todo >= SPLIT_MIN_MOVES_TODO) {
    Task *task;

    if (GetHelper(parent_, this, &move)) {
      return true;
    } else if ((task = search_->tasks->GetIdleTask()) != nullptr) {
      task->node_ = this;
      task->move_ = &move;
      task->search_->Clone(search_);

      {
        std::unique_lock lock(mutex_);

        slaves_.push_back(task->search_);
      }

      {
        std::unique_lock lock(task->mutex_);

        task->run_ = true;

        lock.unlock();
        task->cv_.notify_all();
      }

      return true;
    }
  }

  return false;
}

void Node::WaitSlaves() {
  std::unique_lock lock(mutex_);

  if (alpha >= beta || search_->stop) {
    for (Search *search : slaves_) {
      search->StopAll(STOP_PARALLEL);
    }
  }

  while (slaves_.size()) {
    waiting_ = true;

    assert(helping_ == false);

    cv_.wait(lock);

    if (helping_) {
      assert(help_->run_);

      help_->Search();
      delete help_;
      helping_ = false;
    } else {
      waiting_ = false;
    }
  }

  if (search_->stop == STOP_PARALLEL && stop_point_) {
    search_->stop = RUNNING;
    stop_point_ = false;
  }
}

Move *Node::FirstMove(MoveList &move_list) {
  Move *move = nullptr;
  std::unique_lock lock(mutex_);

  moves_done = 0;
  moves_todo = move_list.size();
  move_ = move_list.data();

  if (move_ && !search_->stop) {
    assert(alpha < beta);
    move = move_;
  }

  lock.unlock();

  return move;
}

Move *Node::NextMove() {
  std::unique_lock lock(mutex_);

  Move *move = NextMoveLockless();

  lock.unlock();

  return move;
}

Move *Node::NextMoveLockless() {
  Move *move = nullptr;

  if (move_ && alpha < beta && !search_->stop && moves_todo > 1) {
    ++moves_done;
    --moves_todo;

    move = ++move_;
  }

  return move;
}

void Node::Update(Move &move) {
  std::unique_lock lock(mutex_);

  if (!search_->stop && move.score > best_score_) {
    best_score_ = move.score;
    best_move_ = move;

    if (height_ == 0) {
      search_->RecordBestMove(move, alpha, beta, depth);
      --search_->result_->moves_left_;
    }

    if (best_score_ > alpha) {
      alpha = best_score_;
    }
  }

  // INFO: stop slaves
  if (alpha >= beta) {
    for (Search *search : slaves_) {
      search->StopAll(STOP_PARALLEL);
    }
  }

  lock.unlock();
}

bool GetHelper(Node *master, Node *node, Move *move) {
  if (!master) {
    return false;
  }

  if (master->waiting_ && !master->helping_) {
    std::unique_lock lock(master->mutex_);

    if (master->slaves_.size() && master->waiting_ && !master->helping_) {
      master->helping_ = true;
      Task *task = master->help_ == nullptr ? new Task() : master->help_;

      task->node_ = node;
      task->move_ = move;
      task->search_->Clone(node->search_);

      std::unique_lock lock(node->mutex_);

      node->slaves_.push_back(task->search_);

      task->run_ = true;

      lock.unlock();
      task->cv_.notify_all();

      return true;
    }
  }

  return GetHelper(master->parent_, node, move);
}

Task::Task()
    : run_(false),
      stop_(false),
      helping_(false),
      search_(new class Search),
      thread_(&Task::Loop, this) {
  search_->task_ = this;
}

Task::~Task() {
  std::unique_lock lock(mutex_);

  assert(run_ == false);

  stop_ = true;

  lock.unlock();
  cv_.notify_all();

  thread_.join();

  std::free(search_);
}

void Task::Loop() {
  std::unique_lock lock(mutex_);

  while (!stop_) {
    cv_.wait(lock, [this] { return stop_ || run_; });

    if (run_) {
      Search();
      container_->PutIdleTask(this);
    }
  }
}

void Task::Search() {
  search_->SetState(node_->search_->stop);

  while (move_ && !search_->stop) {
    const int alpha = node_->alpha;

    if (alpha >= node_->beta) {
      break;
    }

    // search middlegame
    move_->score =
        -search_->Midgame(*move_, -alpha - 1, search_->depth - 1, node_);

    // TODO: maybe use a score stabilizer & log moves
    if (node_->height_ == 0) {
    }

    std::unique_lock lock(node_->mutex_);

    if (!search_->stop && move_->score > node_->best_score_) {
      node_->best_score_ = move_->score;
      node_->best_move_ = *move_;

      // TODO: record best move & log pv line
      if (node_->height_ == 0) {
        search_->RecordBestMove(*move_, alpha, node_->beta, node_->depth);
        --search_->result_->moves_left_;
      }

      if (node_->best_score_ > node_->alpha) {
        node_->alpha = node_->best_score_;

        // stop the master thread?
        if (node_->alpha >= node_->beta && node_->search_->stop == RUNNING) {
          node_->stop_point_ = true;
          node_->search_->stop = STOP_PARALLEL;
        }
      }
    }

    // next move
    move_ = node_->NextMoveLockless();
  }

  search_->SetState(STOP_END);

  search_->parent_->spin_.Lock();

  std::erase(search_->parent_->children_, search_);

  search_->parent_->spin_.Unlock();

  std::unique_lock lock(node_->mutex_);

  run_ = false;

  std::erase(node_->slaves_, search_);

  lock.unlock();
  node_->cv_.notify_all();
}

TaskStack::TaskStack(int size)
    : size_(size), idle_(0), tasks_(size_), stack_(size_, nullptr) {
  for (Task &task : tasks_) {
    task.container_ = this;

    PutIdleTask(&task);
  }
}

void TaskStack::PutIdleTask(Task *task) {
  std::unique_lock lock(mutex_);

  stack_[idle_++] = task;
}

Task *TaskStack::GetIdleTask() {
  std::unique_lock lock(mutex_);

  return idle_ ? stack_[--idle_] : nullptr;
}
