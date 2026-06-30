/**
 * @file TaskScheduler.cc
 * @brief Task scheduler implementation
 */

#include <cppforge/starter/scheduler/TaskScheduler.hpp>
#include <glog/logging.h>

namespace cppforge::starter::scheduler
{
    TaskScheduler::TaskScheduler(std::size_t thread_pool_size)
        : thread_pool_size_(thread_pool_size)
    {
    }

    TaskScheduler::~TaskScheduler()
    {
        stop();
    }

    TaskId TaskScheduler::schedule(const std::string& cron_expr, std::function<void()> task)
    {
        auto id = next_id_.fetch_add(1);
        auto cron = CronExpression(cron_expr);
        auto now = std::chrono::system_clock::now();
        auto next = cron.nextExecutionTime(now);

        auto scheduled = std::make_shared<ScheduledTask>();
        scheduled->id = id;
        scheduled->next_execution = next;
        scheduled->task = std::move(task);
        scheduled->cron = std::move(cron);
        scheduled->periodic = true;

        {
            std::lock_guard<std::mutex> lock(mutex_);
            task_queue_.push(scheduled);
        }
        cv_.notify_one();

        LOG(INFO) << "Scheduled cron task " << id << " with expression '" << cron_expr << "'";
        return id;
    }

    TaskId TaskScheduler::scheduleOnce(std::chrono::milliseconds delay, std::function<void()> task)
    {
        auto id = next_id_.fetch_add(1);
        auto next = std::chrono::system_clock::now() + delay;

        auto scheduled = std::make_shared<ScheduledTask>();
        scheduled->id = id;
        scheduled->next_execution = next;
        scheduled->task = std::move(task);
        scheduled->periodic = false;

        {
            std::lock_guard<std::mutex> lock(mutex_);
            task_queue_.push(scheduled);
        }
        cv_.notify_one();

        LOG(INFO) << "Scheduled one-time task " << id << " with delay " << delay.count() << "ms";
        return id;
    }

    TaskId TaskScheduler::schedulePeriodic(std::chrono::milliseconds interval, std::function<void()> task)
    {
        auto id = next_id_.fetch_add(1);
        auto next = std::chrono::system_clock::now() + interval;

        auto scheduled = std::make_shared<ScheduledTask>();
        scheduled->id = id;
        scheduled->next_execution = next;
        scheduled->task = std::move(task);
        scheduled->interval = interval;
        scheduled->periodic = true;

        {
            std::lock_guard<std::mutex> lock(mutex_);
            task_queue_.push(scheduled);
        }
        cv_.notify_one();

        LOG(INFO) << "Scheduled periodic task " << id << " with interval " << interval.count() << "ms";
        return id;
    }

    bool TaskScheduler::cancel(TaskId task_id)
    {
        std::lock_guard<std::mutex> lock(mutex_);

        // We cannot efficiently remove from a priority_queue, so we mark as cancelled.
        // The worker will skip cancelled tasks when it encounters them.
        // For a production implementation, a more efficient data structure would be used.
        // We need to iterate through the underlying container.
        auto& container = task_queue_.get_container();
        for (auto& task : container)
        {
            if (task->id == task_id)
            {
                task->cancelled = true;
                LOG(INFO) << "Cancelled task " << task_id;
                return true;
            }
        }
        return false;
    }

    void TaskScheduler::start()
    {
        if (running_.exchange(true))
        {
            return; // Already running
        }

        LOG(INFO) << "Starting TaskScheduler with " << thread_pool_size_ << " worker threads";

        for (std::size_t i = 0; i < thread_pool_size_; ++i)
        {
            workers_.emplace_back(&TaskScheduler::workerLoop, this);
        }
    }

    void TaskScheduler::stop()
    {
        if (!running_.exchange(false))
        {
            return; // Already stopped
        }

        cv_.notify_all();

        for (auto& worker : workers_)
        {
            if (worker.joinable())
            {
                worker.join();
            }
        }
        workers_.clear();

        LOG(INFO) << "TaskScheduler stopped";
    }

    bool TaskScheduler::isRunning() const
    {
        return running_.load();
    }

    void TaskScheduler::workerLoop()
    {
        while (running_.load())
        {
            std::unique_lock<std::mutex> lock(mutex_);

            if (task_queue_.empty())
            {
                cv_.wait_for(lock, std::chrono::milliseconds(100));
                continue;
            }

            auto next_task = task_queue_.top();
            auto now = std::chrono::system_clock::now();

            if (next_task->cancelled)
            {
                task_queue_.pop();
                continue;
            }

            if (next_task->next_execution <= now)
            {
                task_queue_.pop();
                lock.unlock();

                try
                {
                    next_task->task();
                }
                catch (const std::exception& e)
                {
                    LOG(ERROR) << "Task " << next_task->id << " threw exception: " << e.what();
                }

                // Re-schedule if periodic with cron or interval, only if still running
                if (next_task->periodic && !next_task->cancelled && running_.load())
                {
                    if (next_task->cron.has_value())
                    {
                        next_task->next_execution = next_task->cron->nextExecutionTime(std::chrono::system_clock::now());
                    }
                    else if (next_task->interval.count() > 0)
                    {
                        next_task->next_execution = std::chrono::system_clock::now() + next_task->interval;
                    }

                    lock.lock();
                    task_queue_.push(next_task);
                    cv_.notify_one();
                }
            }
            else
            {
                // Wait until the next task is due or a new task is added
                cv_.wait_for(lock, next_task->next_execution - now);
            }
        }
    }
}