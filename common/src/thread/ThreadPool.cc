/**
 * @file ThreadPool.cc
 * @brief ThreadPool implementation â€?worker loop, task queue, shutdown
 * @details Implements the thread pool: worker threads loop popping tasks from
 *          a BlockingQueue, enqueue() submits tasks packaged as std::packaged_task,
 *          and shutdown joins all threads after completing pending work.
 */

#include <cppforge/thread/ThreadPool.hpp>

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <functional>
#include <future>
#include <queue>
#include <stdexcept>
#include <thread>
#include <type_traits>
#include <vector>
#include <fmt/format.h>
#include <glog/logging.h>

namespace cppforge::thread
{
    ThreadPool::ThreadPool(const size_t core_threads, const size_t max_threads, const size_t queue_size, const std::chrono::milliseconds idle_time) : core_thread_count_(core_threads), max_thread_count_(max_threads), max_queue_size_(queue_size), thread_idle_time_(idle_time)
    {
        if (core_threads == 0)
        {
            DLOG(WARNING) << "ThreadPool constructor: core_threads must be greater than 0";
            throw std::invalid_argument("ThreadPool::ThreadPool: core_threads must be greater than 0");
        }

        if (max_threads < core_threads)
        {
            DLOG(WARNING) << fmt::format("ThreadPool constructor: max_threads ({}) < core_threads ({})", max_threads, core_threads);
            throw std::invalid_argument("ThreadPool::ThreadPool: max_threads cannot be less than core_threads");
        }

        if (queue_size == 0)
        {
            DLOG(WARNING) << "ThreadPool constructor: queue_size must be greater than 0";
            throw std::invalid_argument("ThreadPool::ThreadPool: queue_size must be greater than 0");
        }

        for (size_t i = 0; i < core_thread_count_; ++i)
        {
            addWorker();
        }
    }

    ThreadPool::~ThreadPool()
    {
        if (!stop_)
        {
            shutdown();
        }
    }

    void ThreadPool::shutdown()
    {
        {
            std::unique_lock lock(queue_mutex_);
            if (stop_) return; // Already stopped
            stop_ = true;
        }
        condition_.notify_all();
        for (std::thread& worker : workers_)
        {
            if (worker.joinable()) worker.join();
        }
    }

    void ThreadPool::shutdownNow()
    {
        {
            std::unique_lock lock(queue_mutex_);
            if (stop_) return; // Already stopped
            stop_ = true;
            // Clear the task queue
            std::queue<std::function<void()>> empty_queue;
            task_queue_.swap(empty_queue); // Clear the queue efficiently
        }
        condition_.notify_all();
        for (std::thread& worker : workers_)
        {
            if (worker.joinable()) worker.join();
        }
    }

    size_t ThreadPool::getActiveThreadCount() const
    {
        return active_thread_count_.load();
    }

    size_t ThreadPool::getQueueSize()
    {
        std::unique_lock lock(queue_mutex_);
        return task_queue_.size();
    }

    void ThreadPool::worker()
    {
        while (true)
        {
            std::function<void()> task;
            {
                std::unique_lock lock(queue_mutex_);
                // Wait for a task or until timeout occurs for non-core threads
                condition_.wait_for(lock, thread_idle_time_, [this]
                {
                    return stop_ || !task_queue_.empty();
                });

                // Exit if shutting down and no more tasks
                if (stop_ && task_queue_.empty())
                {
                    return;
                }

                // For non-core threads, exit if queue is empty, and we have more than core count
                if (task_queue_.empty() && active_thread_count_ > core_thread_count_)
                {
                    --active_thread_count_;
                    return;
                }

                // Get a task if available
                if (!task_queue_.empty())
                {
                    task = std::move(task_queue_.front());
                    task_queue_.pop();
                }
            }
            if (task)
            {
                try
                {
                    task();
                }
                catch (...)
                {
                    // Silently handle exceptions to prevent worker termination
                }
            }
        }
    }

    bool ThreadPool::addWorker()
    {
        if (active_thread_count_ >= max_thread_count_)
        {
            return false;
        }
        ++active_thread_count_;
        workers_.emplace_back([this]
        {
            worker();
        });
        return true;
    }
}
