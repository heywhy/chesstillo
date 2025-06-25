#include <algorithm>
#include <cstddef>
#include <cstdlib>
#include <mutex>
#include <thread>

#include <engine/scheduler.hpp>

using namespace std::chrono_literals;

namespace engine {

int Worker::ID = 1;

Scheduler::Scheduler() : Scheduler(std::thread::hardware_concurrency()) {}

Scheduler::Scheduler(int workers)
    : size_(workers), ready_(false), stopped_(false), workers_(size_) {}

void Scheduler::Init() {
  std::unique_lock lock(mutex_);

  for (Worker &worker : workers_) {
    worker.Init(this);
  }

  thread_ = std::thread(&Scheduler::Monitor, this);

  cv_.wait(lock, [this] { return ready_; });
  lock.unlock();
}

void Scheduler::Monitor() {
  std::unique_lock lock(mutex_);

  ready_ = true;

  lock.unlock();
  cv_.notify_all();

  while (true) {
    lock.lock();

    if (stopped_) {
      lock.unlock();
      break;
    }

    cv_.wait(lock, [this] { return !free_.empty() || stopped_; });

    while (!free_.empty()) {
      Worker *worker = free_.front();

      free_.pop();

      if (pending_.empty()) {
        busy_.erase(worker->id_);
        continue;
      }

      Job *job = pending_.front();

      pending_.pop();
      worker->Run(job);
    }

    lock.unlock();
    cv_.notify_all();
  }
}

void Scheduler::Stop() {
  std::unique_lock lock(mutex_);

  if (stopped_) {
    lock.unlock();
    return;
  }

  // INFO: unlock the mutex so that any busy worker can call the scheduler
  // without leading to a deadlock
  lock.unlock();

  for (Worker &worker : workers_) {
    worker.Stop();
  }

  lock.lock();

  stopped_ = true;

  lock.unlock();
  cv_.notify_all();
  thread_.join();
}

void Scheduler::MakeAvailable(Worker *worker) {
  std::unique_lock lock(mutex_);

  free_.push(worker);

  lock.unlock();
  cv_.notify_all();
}

std::size_t Scheduler::Busy() {
  std::unique_lock lock(mutex_);

  std::size_t size = busy_.size();

  lock.unlock();

  return size;
}

Status *Scheduler::Dispatch(Callback callback) {
  // NOTE: random selection or even distribution?!
  // TODO: lock worker to confirm actual state. instead of find, maybe push to
  // another container

  std::unique_lock lock(mutex_);

  auto it = std::find_if(workers_.begin(), workers_.end(), [&](Worker &worker) {
    return !worker.stopped_ && !busy_.contains(worker.id_);
  });

  Status *status = new Status(callback);

  if (it != workers_.end()) {
    Worker *worker = &(*it);

    busy_.insert(worker->id_);
    lock.unlock();

    it->Run(&status->job_);

    return status;
  }

  pending_.push(&status->job_);
  lock.unlock();
  cv_.notify_all();

  return status;
}

void Status::Wait() {
  while (!job_.done_) {
    Worker *worker = job_.worker_;

    if (worker == nullptr) {
      continue;
    }

    std::unique_lock lock(worker->mutex_);

    worker->cv_.wait(lock, [this] { return job_.done_; });
    lock.unlock();
  }
}

void Worker::Init(Scheduler *scheduler) {
  std::unique_lock lock(mutex_);

  scheduler_ = scheduler;
  thread_ = std::thread(&Worker::Wait, this);

  // wait for the ready signal
  cv_.wait(lock, [this] { return ready_; });
  lock.unlock();
}

void Worker::Stop() {
  std::unique_lock lock(mutex_);

  if (stopped_) {
    lock.unlock();
    return;
  }

  // set this to true so that so that the thread can stop running
  stopped_ = true;

  lock.unlock();
  cv_.notify_all();
  thread_.join();
}

void Worker::Wait() {
  std::unique_lock lock(mutex_);

  ready_ = true;

  lock.unlock();
  cv_.notify_all();

  while (true) {
    lock.lock();

    if (stopped_) {
      lock.unlock();
      break;
    }

    cv_.wait(lock, [this] { return job_ != nullptr || stopped_; });

    if (job_ != nullptr) {
      job_->Execute();

      job_ = nullptr;

      // there is no need telling the scheduler it's free if it's asked to stop
      // after its last job
      if (!stopped_) {
        scheduler_->MakeAvailable(this);
      }
    }

    lock.unlock();
    cv_.notify_all();
  }
}

void Worker::Run(Job *job) {
  std::unique_lock lock(mutex_);

  job_ = job;
  job_->worker_ = this;

  lock.unlock();
  cv_.notify_all();
}

}  // namespace engine
