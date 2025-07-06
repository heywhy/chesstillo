#include <atomic>

#include <engine/threads.hpp>

SpinLock::SpinLock() : atomic_flag_(ATOMIC_FLAG_INIT) {}

void SpinLock::Lock() {
  while (atomic_flag_.test_and_set(std::memory_order_acquire)) {
  }
}

void SpinLock::Unlock() { atomic_flag_.clear(std::memory_order_release); }
