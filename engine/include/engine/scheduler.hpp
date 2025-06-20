#ifndef SCHEDULER_HPP
#define SCHEDULER_HPP

#include <condition_variable>
#include <cstddef>
#include <cstdlib>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>
#include <unordered_set>
#include <vector>

namespace engine {

typedef std::function<void()> Callback;

class Status;
class Scheduler;
class Worker;

class Job {
 private:
  Callback callback_;
  bool done_;
  Worker *worker_;

  Job(Callback fun) : callback_(fun), done_(false), worker_(nullptr) {}

  void Execute() {
    if (done_) {
      return;
    }

    callback_();
    done_ = true;
    worker_ = nullptr;
  }

  friend class Status;
  friend class Worker;
};

class Worker {
 public:
  Worker() : job_(nullptr), id_(ID++), ready_(false), stopped_(false) {}
  ~Worker() { Stop(); }

 private:
  Job *job_;
  std::mutex mutex_;
  std::thread thread_;
  std::condition_variable cv_;

  int id_;
  bool ready_;
  bool stopped_;
  Scheduler *scheduler_;

  static int ID;

  void Init(Scheduler *scheduler);
  void Stop();
  void Run(Job *job);
  void Wait();

  friend class Scheduler;
  friend class Status;
};

class Status {
 public:
  void Wait();

 private:
  Job job_;

  Status(Callback callback) : job_(callback) {}

  friend class Scheduler;
};

class Scheduler {
 public:
  Scheduler();
  Scheduler(int workers);

  ~Scheduler() { Stop(); }

  size_t Size() { return size_; }

  void Init();
  size_t Busy();
  void MakeAvailable(Worker *worker);
  Status *Dispatch(Callback callback);

 private:
  int size_;
  bool ready_;
  bool stopped_;
  std::queue<Job *> pending_;
  std::vector<Worker> workers_;
  std::queue<Worker *> free_;
  std::unordered_set<int> busy_;

  std::mutex mutex_;
  std::thread thread_;
  std::condition_variable cv_;

  void Stop();
  void Monitor();
};

}  // namespace engine

#endif
