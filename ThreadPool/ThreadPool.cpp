#include "ThreadPool.h"
#include <iostream>
#include <string>

ThreadPool::ThreadPool(unsigned count_of_threads)
{
  auto max_count = std::thread::hardware_concurrency();
  if (max_count != 0 && (count_of_threads == 0 || count_of_threads > max_count))
  {
    count_of_threads = max_count;
  }
  else if (max_count == 0 && count_of_threads == 0)
  {
    count_of_threads = 2;
  }
  if (!count_of_threads) throw std::invalid_argument("Thread pool construction failed");
  m_threads.reserve(count_of_threads);
  for (unsigned i = 0; i < count_of_threads; ++i)
  {
    m_threads.emplace_back(&ThreadPool::RunExecution, this);
  }
}

ThreadPool::~ThreadPool()
{
  std::unique_lock<std::mutex> lock{ m_queue_mutex };
  m_stop_flag = true;
  lock.unlock();

  m_notifier.notify_all();

  for (auto& thread : m_threads)
    thread.join();
}

void ThreadPool::Wait(int msec_wait_interval)
{
  while (m_active_threads_count && !m_available_tasks.empty())
  {
    std::this_thread::sleep_for(std::chrono::milliseconds{ msec_wait_interval });
  }
}


void ThreadPool::RunExecution()
{
  bool initialized = false;
  while (true)
  {
    std::unique_lock<std::mutex> lock{ m_queue_mutex };
    if (initialized && m_available_tasks.empty() && m_active_threads_count == 0)
    {
      m_finish_indicator.notify_one();
    }
    initialized = true;

    m_notifier.wait(lock, [this] {return m_stop_flag || !m_available_tasks.empty(); });
    if (!m_stop_flag)
    {
      ++m_active_threads_count;
      auto current_task = std::move(m_available_tasks.front());
      m_available_tasks.pop();
      lock.unlock();
      try
      {
        current_task();
      }
      catch (const std::exception& exception)
      {
        std::cerr << "Task execution failed with message :" << exception.what();
        return;
      }
      --m_active_threads_count;
    }
    else return;
  }
}