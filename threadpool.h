#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <condition_variable>
#include <functional>
#include <iostream>
#include <mutex>
#include <queue>
#include <thread>

class ThreadPool {
public:
  ThreadPool(size_t workers_count = std::thread::hardware_concurrency() - 1);
  ~ThreadPool();
  void run();
  void add_work(std::function<void()> &&task);

private:
  std::vector<std::thread> workers;
  std::mutex mtx;
  std::deque<std::function<void()>> tasks;
  std::condition_variable cv;
  bool stopped = false;
};

#endif // THREADPOOL_H
