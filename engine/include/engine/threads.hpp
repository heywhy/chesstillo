#ifndef ENGINE_THREADS_HPP
#define ENGINE_THREADS_HPP

#include <atomic>

class SpinLock {
 public:
  void Lock();
  void Unlock();

 private:
  std::atomic_flag atomic_flag_ = ATOMIC_FLAG_INIT;
};

#endif
