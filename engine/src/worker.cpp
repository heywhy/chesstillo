#include <cassert>
#include <cstddef>
#include <mutex>
#include <thread>

#include <engine/search.hpp>

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

Worker::Worker()
    : loop_(true),
      node_(nullptr),
      nodes_(0),
      search_(new class Search(nullptr)) {
  std::lock_guard lock(mutex_);

  thread_ = std::thread(&Worker::Loop, this);
}

Worker::~Worker() {
  std::unique_lock lock(mutex_);

  assert(loop_);

  loop_ = false;

  lock.unlock();
  cv_.notify_all();

  thread_.join();

  delete search_;
}

void Worker::Assign(Node *node) {
  std::unique_lock lock(mutex_);

  assert(node_ == nullptr);

  ++nodes_;
  node_ = node;

  lock.unlock();
  cv_.notify_all();
}

void Worker::Search() { node_ = nullptr; }

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
