#include "src/thread/ThreadPool.hpp"

#include <glog/logging.h>
#include <fmt/format.h>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <functional>
#include <future>
#include <queue>
#include <thread>
#include <vector>
#include <stdexcept>
#include <type_traits>
#include <sstream>

namespace common::thread {
    namespace {
        // Helper function to convert thread::id to string
        auto getThreadIdString() -> std::string {
            std::ostringstream oss;
            oss << std::this_thread::get_id();
            return oss.str();
        }
    }

    ThreadPool::ThreadPool(const size_t core_threads, const size_t max_threads, const size_t queue_size, const std::chrono::milliseconds idle_time) : core_thread_count_(core_threads), max_thread_count_(max_threads), max_queue_size_(queue_size), thread_idle_time_(idle_time) {
        if (core_threads == 0) {
            DLOG(ERROR) << fmt::format("ThreadPool initialization failed - core_threads is 0");
            throw std::invalid_argument("ThreadPool::ThreadPool: core_threads must be greater than 0");
        }

        if (max_threads < core_threads) {
            DLOG(ERROR) << fmt::format("ThreadPool initialization failed - max_threads({}) < core_threads({})", max_threads, core_threads);
            throw std::invalid_argument("ThreadPool::ThreadPool: max_threads cannot be less than core_threads");
        }

        if (queue_size == 0) {
            DLOG(ERROR) << fmt::format("ThreadPool initialization failed - queue_size is 0");
            throw std::invalid_argument("ThreadPool::ThreadPool: queue_size must be greater than 0");
        }

        DLOG(INFO) << fmt::format("ThreadPool initializing - core_threads={}, max_threads={}, queue_size={}, idle_time={}ms",
                                  core_threads, max_threads, queue_size, idle_time.count());

        for (size_t i = 0; i < core_thread_count_; ++i) {
            addWorker();
        }

        DLOG(INFO) << fmt::format("ThreadPool initialized successfully with {} core workers", core_thread_count_.load());
    }

    ThreadPool::~ThreadPool() {
        if (!stop_) {
            shutdown();
        }
    }

    auto ThreadPool::shutdown() -> void {
        DLOG(INFO) << "ThreadPool shutdown - initiating graceful shutdown";
        {
            std::unique_lock lock(queue_mutex_);
            if (stop_) return; // Already stopped
            stop_ = true;
        }
        condition_.notify_all();
        for (std::thread &worker: workers_) {
            if (worker.joinable()) worker.join();
        }
        DLOG(INFO) << fmt::format("ThreadPool shutdown completed - all {} workers joined", workers_.size());
    }

    auto ThreadPool::shutdownNow() -> void {
        DLOG(WARNING) << "ThreadPool shutdownNow - initiating immediate shutdown";
        {
            std::unique_lock lock(queue_mutex_);
            if (stop_) return; // Already stopped
            stop_ = true;
            // Clear the task queue
            const size_t discarded_tasks = task_queue_.size();
            std::queue<std::function<void()> > empty_queue;
            task_queue_.swap(empty_queue); // Clear the queue efficiently
            if (discarded_tasks > 0) {
                DLOG(WARNING) << fmt::format("ThreadPool shutdownNow - discarded {} pending tasks", discarded_tasks);
            }
        }
        condition_.notify_all();
        for (std::thread &worker: workers_) {
            if (worker.joinable()) worker.join();
        }
        DLOG(INFO) << fmt::format("ThreadPool shutdownNow completed - all {} workers joined", workers_.size());
    }

    auto ThreadPool::getActiveThreadCount() const -> size_t {
        return active_thread_count_.load();
    }

    auto ThreadPool::getQueueSize() -> size_t {
        std::unique_lock lock(queue_mutex_);
        return task_queue_.size();
    }

    auto ThreadPool::worker() -> void {
        DLOG(INFO) << fmt::format("ThreadPool worker started - thread_id={}", getThreadIdString());
        while (true) {
            std::function<void()> task;
            {
                std::unique_lock lock(queue_mutex_);
                // Wait for a task or until timeout occurs for non-core threads
                condition_.wait_for(lock, thread_idle_time_, [this] {
                    return stop_ || !task_queue_.empty();
                });

                // Exit if shutting down and no more tasks
                if (stop_ && task_queue_.empty()) {
                    DLOG(INFO) << fmt::format("ThreadPool worker exiting - shutdown requested, thread_id={}", getThreadIdString());
                    return;
                }

                // For non-core threads, exit if queue is empty and we have more than core count
                if (task_queue_.empty() && active_thread_count_ > core_thread_count_) {
                    --active_thread_count_;
                    DLOG(INFO) << fmt::format("ThreadPool worker exiting - excess thread, thread_id={}", getThreadIdString());
                    return;
                }

                // Get a task if available
                if (!task_queue_.empty()) {
                    task = std::move(task_queue_.front());
                    task_queue_.pop();
                }
            }
            if (task) {
                try {
                    task();
                } catch (const std::exception &e) {
                    DLOG(ERROR) << fmt::format("ThreadPool worker task exception - error: {}, thread_id={}", e.what(), getThreadIdString());
                } catch (...) {
                    DLOG(ERROR) << fmt::format("ThreadPool worker task unknown exception - thread_id={}", getThreadIdString());
                }
            }
        }
    }

    auto ThreadPool::addWorker() -> bool {
        if (active_thread_count_ >= max_thread_count_) {
            DLOG(WARNING) << fmt::format("ThreadPool addWorker failed - reached max threads: {}/{}", active_thread_count_.load(), max_thread_count_);
            return false;
        }
        ++active_thread_count_;
        workers_.emplace_back([this] { worker(); });
        DLOG(INFO) << fmt::format("ThreadPool addWorker succeeded - active_threads: {}/{}", active_thread_count_.load(), max_thread_count_);
        return true;
    }
}
