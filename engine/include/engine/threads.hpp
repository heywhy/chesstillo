#ifndef THREADS_HPP
#define THREADS_HPP

#include <atomic>

class SpinLock {
 public:
  SpinLock();

  void Lock();
  void Unlock();

 private:
  std::atomic_flag atomic_flag_;
};

#endif
