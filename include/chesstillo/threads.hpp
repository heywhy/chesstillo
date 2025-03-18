#ifndef THREADS_HPP
#define THREADS_HPP

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <thread>
#include <vector>

#include <chesstillo/types.hpp>

class SpinLock {
public:
  void Lock();
  void Unlock();

private:
  std::atomic_flag atomic_flag_ = ATOMIC_FLAG_INIT;
};

class Slave {
public:
  Slave();
  ~Slave();

  int GetId();
  bool Idle();
  void Stop();
  bool Assign(SplitPoint *work);

private:
  static int ID;

  int id_;
  bool ready_;
  bool stopped_;

  SplitPoint *work_;

  std::mutex mutex_;
  std::thread thread_;
  std::condition_variable cv_;

  void Loop();
};

class SlaveManager {
public:
  SlaveManager() : slaves_(std::thread::hardware_concurrency()) {};
  ~SlaveManager() { Stop(); }

  Slave *FindIdleSlave();

private:
  std::vector<Slave> slaves_;

  void Stop();
};

#endif
