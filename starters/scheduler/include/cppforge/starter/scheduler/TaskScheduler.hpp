/**
 * @file TaskScheduler.hpp
 * @brief Thread-safe task scheduler with cron, delayed, and periodic scheduling
 * @details Provides scheduling capabilities for recurring (cron-based), one-time delayed,
 *          and fixed-interval tasks. Uses a thread pool and condition variable for timing.
 *
 * @par Thread Safety
 * This class is thread-safe. All public methods can be called from any thread.
 */

#pragma once
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <vector>

#include <cppforge/starter/scheduler/CronExpression.hpp>

namespace cppforge::starter::scheduler
{
    /// @brief Unique identifier for a scheduled task
    using TaskId = uint64_t;

    /// @brief Thread-safe task scheduler supporting cron, delayed, and periodic tasks
    class TaskScheduler
    {
    public:
        /// @brief Construct a task scheduler with the given thread pool size
        /// @param thread_pool_size Number of worker threads
        explicit TaskScheduler(std::size_t thread_pool_size = 4);

        /// @brief Destructor ¡ª stops the scheduler and joins all threads
        ~TaskScheduler();

        // Non-copyable, non-movable
        TaskScheduler(const TaskScheduler&) = delete;
        TaskScheduler& operator=(const TaskScheduler&) = delete;
        TaskScheduler(TaskScheduler&&) = delete;
        TaskScheduler& operator=(TaskScheduler&&) = delete;

        /// @brief Schedule a recurring task based on a cron expression
        /// @param cron_expr Cron expression string (e.g. "*/5 * * * *")
        /// @param task The task to execute
        /// @return Task ID for cancellation
        [[nodiscard]] TaskId schedule(const std::string& cron_expr, std::function<void()> task);

        /// @brief Schedule a one-time delayed task
        /// @param delay Delay before execution
        /// @param task The task to execute
        /// @return Task ID for cancellation
        [[nodiscard]] TaskId scheduleOnce(std::chrono::milliseconds delay, std::function<void()> task);

        /// @brief Schedule a fixed-interval recurring task
        /// @param interval Interval between executions
        /// @param task The task to execute
        /// @return Task ID for cancellation
        [[nodiscard]] TaskId schedulePeriodic(std::chrono::milliseconds interval, std::function<void()> task);

        /// @brief Cancel a scheduled task
        /// @param task_id The ID of the task to cancel
        /// @return true if the task was found and cancelled
        bool cancel(TaskId task_id);

        /// @brief Start the scheduler (begins processing scheduled tasks)
        void start();

        /// @brief Stop the scheduler and cancel all pending tasks
        void stop();

        /// @brief Check if the scheduler is running
        /// @return true if the scheduler is active
        [[nodiscard]] bool isRunning() const;

    private:
        /// @brief Internal representation of a scheduled task
        struct ScheduledTask
        {
            TaskId id;
            std::chrono::system_clock::time_point next_execution;
            std::function<void()> task;
            std::optional<CronExpression> cron;
            std::chrono::milliseconds interval{0};
            bool periodic = false;
            bool cancelled = false;
        };

        /// @brief Comparator for the priority queue (earliest execution first)
        struct TaskComparator
        {
            bool operator()(const std::shared_ptr<ScheduledTask>& a,
                            const std::shared_ptr<ScheduledTask>& b) const
            {
                return a->next_execution > b->next_execution;
            }
        };

        /// @brief Worker thread function
        void workerLoop();

        /// @brief Execute ready tasks from the queue
        void executeReadyTasks();

        std::size_t thread_pool_size_;
        std::atomic<bool> running_{false};
        std::atomic<TaskId> next_id_{1};

        std::mutex mutex_;
        std::condition_variable cv_;
        std::priority_queue<std::shared_ptr<ScheduledTask>,
                            std::vector<std::shared_ptr<ScheduledTask>>,
                            TaskComparator> task_queue_;
        std::vector<std::thread> workers_;
    };
}
