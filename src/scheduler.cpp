#include <algorithm>
#include <cstddef>
#include <cstdlib>
#include <mutex>
#include <thread>
#include <vector>

#include <chesstillo/scheduler.hpp>

using namespace std::chrono_literals;

void Scheduler::Init() {
  for (Worker &worker : workers_) {
    worker.Init(this);
  }

  std::unique_lock lock(mutex_);
  cleaner_ = std::thread(&Scheduler::Monitor, this);

  ready_cv_.wait(lock);
  lock.unlock();

  cleaner_.detach();
}

void Scheduler::Monitor() {
  ready_cv_.notify_all();

  while (true) {
    if (stopped_) {
      break;
    }

    std::unique_lock lock(mutex_);

    cv_.wait(lock);

    busy_.erase(finished_);

    lock.unlock();
    cv_.notify_all();
  }
}

void Scheduler::Stop() {
  if (stopped_) {
    return;
  }

  std::unique_lock lock(mutex_);

  stopped_ = true;

  cv_.notify_all();
  cv_.wait(lock);
  lock.unlock();
}

void Scheduler::MakeAvailable(Worker *worker) {
  std::unique_lock lock(mutex_);

  finished_ = worker;

  lock.unlock();
  cv_.notify_all();
}

Status Scheduler::Dispatch(Callback callback) {
  // NOTE: random selection or even distribution?!
  // TODO: lock worker to confirm actual state. instead of find, maybe push to
  // another container
  auto it = std::find_if(workers_.begin(), workers_.end(), [&](Worker &worker) {
    return !busy_.contains(&worker);
  });

  if (it != workers_.end()) {
    Worker *worker = &(*it);
    Status status(worker, this);

    busy_.insert(worker);

    it->Run(callback);

    return status;
  }

  return {NULL, NULL};
}

void Status::Wait() {
  if (worker_ == nullptr) {
    return;
  }

  if (worker_->callback_ == NULL || worker_->stopped_) {
    worker_ = nullptr;

    return;
  }

  std::unique_lock lock(worker_->mutex_);

  worker_->cv_.wait(lock);
  lock.unlock();

  worker_ = nullptr;
}

void Worker::Init(Scheduler *scheduler) {
  scheduler_ = scheduler;
  std::unique_lock lock(mutex_);
  thread_ = std::thread(&Worker::Wait, this);

  // wait for the ready signal
  ready_cv_.wait(lock);
  lock.unlock();

  // detach so that it can run independent of the main thread
  thread_.detach();
}

void Worker::Stop() {
  if (stopped_) {
    return;
  }

  std::unique_lock lock(mutex_);

  // set this to true so that so that the thread can stop running
  stopped_ = true;

  cv_.notify_all();
  cv_.wait(lock);
  lock.unlock();
}

void Worker::Wait() {
  ready_cv_.notify_all();

  while (true) {
    if (stopped_) {
      break;
    }

    std::unique_lock lock(mutex_);

    cv_.wait(lock);

    if (callback_ != NULL) {
      callback_();
      callback_ = NULL;

      if (scheduler_ != nullptr) {
        scheduler_->MakeAvailable(this);
      }
    }

    lock.unlock();
    cv_.notify_all();
  }
}

void Worker::Run(Callback callback) {
  std::unique_lock lock(mutex_);

  callback_ = callback;

  lock.unlock();
  cv_.notify_all();
}
