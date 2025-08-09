#include <cassert>
#include <cstddef>
#include <mutex>
#include <thread>

#include "engine/move.hpp"
#include "engine/search.hpp"
#include "engine/types.hpp"

namespace engine {
namespace search {
WorkerRegistry::WorkerRegistry(std::size_t count)
    : idle_(count), workers_(count), stack_(count) {
  for (std::size_t i = 0; i < idle_; i++) {
    auto worker = &workers_[i];

    worker->registry = this;

    stack_[i] = worker;
  }
}

std::size_t WorkerRegistry::IdleWorkers() {
  std::lock_guard lock(mutex_);

  return idle_;
}

Worker *WorkerRegistry::GetIdleWorker() {
  std::lock_guard lock(mutex_);

  return idle_ ? stack_[--idle_] : nullptr;
}

void WorkerRegistry::PutIdleWorker(Worker *worker) {
  std::lock_guard lock(mutex_);

  stack_[idle_++] = worker;
}

Worker::Worker(bool loop)
    : loop_(loop), node_(nullptr), nodes_(0), search_(nullptr) {
  std::lock_guard lock(mutex_);

  search_.position = &position_;
  thread_ = std::thread(&Worker::Loop, this);
}

Worker::~Worker() {
  std::unique_lock lock(mutex_);

  loop_ = false;

  lock.unlock();
  cv_.notify_all();

  thread_.join();

  assert(node_ == nullptr);
}

void Worker::Assign(Node *node, Move *move) {
  std::unique_lock lock(mutex_);

  assert(node_ == nullptr);

  ++nodes_;

  node_ = node;
  move_ = move;

  search_.Clone(node->search);

  node_->AddSlave(&search_);

  lock.unlock();
  cv_.notify_all();
}

void Worker::Search() {
  search_.SetState(node_->search->state);

  while (move_ && search_.state == search::State::RUNNING) {
    const int alpha = node_->alpha;

    if (alpha >= node_->beta) {
      break;
    }

    search_.position->Make(*move_);

    move_->score = -search_.NW_Search(alpha, node_->depth - 1, node_);

    if (alpha < move_->score && move_->score < node_->beta) {
      move_->score = search_.search<NodeType::PV>(-node_->beta, -node_->alpha,
                                                  node_->depth - 1, node_);

      assert(node_->type == NodeType::PV);
    }

    search_.position->Undo(*move_);

    // TODO: send info
    std::lock_guard lock(node_->mutex_);

    if (search_.state == search::State::RUNNING &&
        move_->score > node_->best_score) {
      node_->best_score = move_->score;
      node_->best_move = *move_;

      // TODO: record best move?

      if (node_->best_score > node_->alpha) {
        node_->alpha = node_->best_score;

        if (node_->alpha >= node_->beta &&
            node_->search->state == search::State::RUNNING) {
          node_->stop_point_ = true;
          node_->search->state = search::State::STOP_PARALLEL;
        }
      }
    }

    move_ = node_->NextMoveLockless();
  }

  search_.SetState(search::State::END);

  search_.Detach();
  node_->RemoveSlave(&search_);

  node_ = nullptr;
  move_ = nullptr;
}

void Worker::Loop() {
  std::unique_lock lock(mutex_);

  while (loop_) {
    cv_.wait(lock, [&] { return !loop_ || node_ != nullptr; });

    if (node_) {
      Search();

      assert(registry != nullptr);

      registry->PutIdleWorker(this);
    }
  }
}
}  // namespace search
}  // namespace engine
