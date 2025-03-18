#include <atomic>
#include <mutex>

#include <chesstillo/search.hpp>
#include <chesstillo/threads.hpp>

void SpinLock::Lock() {
  while (atomic_flag_.test_and_set(std::memory_order_acquire)) {
  }
}

void SpinLock::Unlock() { atomic_flag_.clear(std::memory_order_release); }

int Slave::ID = 1;

Slave::Slave()
    : id_(++ID), ready_(false), stopped_(false), work_(nullptr),
      thread_(&Slave::Loop, this) {}

Slave::~Slave() { Stop(); }

int Slave::GetId() { return id_; }

bool Slave::Idle() {
  std::unique_lock lock(mutex_, std::defer_lock);

  return lock.try_lock() && ready_ && work_ == nullptr;
}

void Slave::Loop() {
  std::unique_lock lock(mutex_);

  ready_ = true;

  lock.unlock();

  while (true) {
    lock.lock();

    if (stopped_) {
      return;
    }

    cv_.wait(lock, [this] { return stopped_ || work_ != nullptr; });

    if (work_ != nullptr && !stopped_) {
      work_->Search(this);

      work_ = nullptr;
    }

    lock.unlock();
    cv_.notify_all();
  }
}

void Slave::Stop() {
  std::unique_lock lock(mutex_);

  if (stopped_) {
    return;
  }

  stopped_ = true;

  lock.unlock();
  cv_.notify_all();
  thread_.join();
}

bool Slave::Assign(SplitPoint *work) {
  std::unique_lock lock(mutex_, std::defer_lock);

  if (!lock.try_lock() || work_ != nullptr) {
    return false;
  }

  work_ = work;

  work_->Tackling(this);

  lock.unlock();
  cv_.notify_all();

  return true;
}

Slave *SlaveManager::FindIdleSlave() {
  for (Slave &slave : slaves_) {
    if (slave.Idle()) {
      return &slave;
    }
  }

  return nullptr;
}

void SlaveManager::Stop() {
  for (Slave &slave : slaves_) {
    slave.Stop();
  }
}
