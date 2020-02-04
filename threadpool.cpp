#include "threadpool.h"


ThreadPool::ThreadPool(size_t workers_count) {
  for (size_t i = 0; i < workers_count; ++i)
    workers.emplace_back(&ThreadPool::run, this);
}

ThreadPool::~ThreadPool() {
  {
    std::scoped_lock<std::mutex> lock(mtx);
    stopped = true;
  }
  cv.notify_all();

  for (auto &i : workers)
    i.join();
}

void ThreadPool::run() {
  std::function<void()> func;
  for (;;) {
    {
      std::unique_lock<std::mutex> lck(mtx);
      cv.wait(lck, [this] { return !tasks.empty() || stopped; });

      if (stopped)
        return;

      func = tasks.front();
      tasks.pop_front();
    }

    try {
      func();
    } catch (std::exception &e) {
      std::cerr << e.what();
      return;
    }
  }
}

void ThreadPool::add_work(std::function<void()> &&task) {
  std::scoped_lock<std::mutex> lck(mtx);
  tasks.push_back(std::move(task));
  cv.notify_one();
}
