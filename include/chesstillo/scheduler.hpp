#ifndef SCHEDULER_HPP
#define SCHEDULER_HPP

#include <condition_variable>
#include <cstddef>
#include <cstdlib>
#include <functional>
#include <mutex>
#include <set>
#include <thread>
#include <vector>

typedef std::function<void()> Callback;

class Scheduler;

class Worker {
public:
  Worker() : callback_(NULL), stopped_(false) {}
  ~Worker() { Stop(); }

private:
  Callback callback_;
  std::mutex mutex_;
  std::thread thread_;
  std::condition_variable cv_;
  std::condition_variable ready_cv_;

  bool stopped_;
  Scheduler *scheduler_;

  void Init(Scheduler *scheduler);
  void Stop();
  void Run(Callback callback);
  void Wait();

  friend class Scheduler;
  friend class Status;
};

class Status {
public:
  bool Done();
  void Wait();

private:
  Worker *worker_;
  void *scheduler_;

  Status(Worker *worker, void *scheduler)
      : worker_(worker), scheduler_(scheduler) {}

  friend class Scheduler;
};

class Scheduler {
public:
  Scheduler(int num) : size_(num), stopped_(false), workers_(size_) {}

  ~Scheduler() { Stop(); }

  size_t Busy() { return busy_.size(); }

  void Init();
  void MakeAvailable(Worker *worker);
  Status Dispatch(Callback callback);

private:
  int size_;
  bool stopped_;
  std::set<Worker *> busy_;
  std::vector<Worker> workers_;

  std::mutex mutex_;
  std::thread cleaner_;
  std::condition_variable cv_;
  std::condition_variable ready_cv_;

  Worker *finished_;

  void Stop();
  void Monitor();
};

#endif
