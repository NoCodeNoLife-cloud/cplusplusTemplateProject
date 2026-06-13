/**
 * @file DelayedTaskActuator.hpp
 * @brief Schedules and executes tasks after a configurable delay
 * @description Executes a given task (callable) once after a specified delay.
 *          Uses a dedicated thread or timer to wait asynchronously without
 *          blocking the caller.  Supports cancellation of pending tasks.
 *
 * @par Thread Safety
 * This class is **not** thread-safe.  External synchronisation is required
 * for concurrent access.
 */

#pragma once
#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <thread>
#include <unordered_map>

namespace common::thread
{
    /// @brief A class that executes tasks with a specified delay.
    /// @tparam ResultType The type of the result returned by the task.
    template <typename ResultType>
    class DelayedTaskActuator
    {
    public:
        DelayedTaskActuator() : state_(std::make_shared<State>())
        {
        }

        /// @brief Destructor marks the actuator as stopped; detached threads
        ///        hold a shared_ptr to the internal state and will not access
        ///        freed memory.
        ~DelayedTaskActuator()
        {
            std::lock_guard lock(state_->mutex);
            state_->alive = false;
            state_->pendingTasks.clear();
            state_->results.clear();
            state_->cv.notify_all();
        }

        DelayedTaskActuator(const DelayedTaskActuator&) = delete;
        DelayedTaskActuator& operator=(const DelayedTaskActuator&) = delete;
        DelayedTaskActuator(DelayedTaskActuator&&) = delete;
        DelayedTaskActuator& operator=(DelayedTaskActuator&&) = delete;

        /// @brief Schedules a task to be executed after a specified delay.
        /// @param delayMs The delay in milliseconds before the task is executed.
        /// @param task The task to be executed.
        /// @return The ID of the scheduled task.
        [[nodiscard]] int32_t scheduleTask(int32_t delayMs, std::function<ResultType()> task);

        /// @brief Retrieves the result of a scheduled task.
        /// @param taskId The ID of the task whose result is to be retrieved.
        /// @return A future object that will hold the result of the task.
        [[nodiscard]] std::future<ResultType> getTaskResult(int32_t taskId);

        /// @brief Checks if a task is currently pending execution.
        /// @param taskId The ID of the task to check.
        /// @return true if the task is pending, false otherwise.
        bool isTaskPending(int32_t taskId) const;

        /// @brief Cancels a scheduled task if it hasn't started yet.
        /// @param taskId The ID of the task to cancel.
        /// @return true if the task was canceled, false if it was not found or already executed.
        [[nodiscard]] bool cancelTask(int32_t taskId);

    private:
        struct State
        {
            std::mutex mutex{};
            std::condition_variable cv{};
            std::unordered_map<int32_t, std::future<ResultType>> results{};
            std::unordered_map<int32_t, std::shared_ptr<bool>> pendingTasks{};
            int32_t nextTaskId{0};
            bool alive{true};
        };

        /// @brief Checks if a task is pending without acquiring the lock (caller must hold mutex).
        static bool isTaskPendingLocked(const std::unordered_map<int32_t, std::shared_ptr<bool>>& pendingTasks, int32_t taskId);

        std::shared_ptr<State> state_;
    };

    template <typename ResultType>
    int32_t DelayedTaskActuator<ResultType>::scheduleTask(int32_t delayMs, std::function<ResultType()> task)
    {
        if (delayMs < 0)
        {
            throw std::invalid_argument("DelayedTaskActuator::scheduleTask: delayMs must be non-negative");
        }

        if (!task)
        {
            throw std::invalid_argument("DelayedTaskActuator::scheduleTask: task function cannot be null");
        }

        // Capture state by shared_ptr so the detached thread keeps it alive
        auto state = state_;
        std::lock_guard lock(state->mutex);
        const int32_t taskId = state->nextTaskId++;

        auto packagedTask = std::make_shared<std::packaged_task<ResultType()>>(std::move(task));
        std::future<ResultType> result = packagedTask->get_future();

        state->pendingTasks[taskId] = std::make_shared<bool>(true);

        std::thread([state, delayMs, packagedTask, taskId]() mutable
        {
            try
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(delayMs));

                std::lock_guard lock(state->mutex);
                if (!state->alive) return;

                (*packagedTask)();

                const auto it = state->pendingTasks.find(taskId);
                if (it != state->pendingTasks.end())
                {
                    *it->second = false;
                    state->pendingTasks.erase(it);
                }
                state->cv.notify_all();
            }
            catch (...)
            {
                std::lock_guard lock(state->mutex);
                if (!state->alive) return;

                const auto it = state->pendingTasks.find(taskId);
                if (it != state->pendingTasks.end())
                {
                    state->pendingTasks.erase(it);
                }
                state->cv.notify_all();
            }
        }).detach();

        state->results[taskId] = std::move(result);
        return taskId;
    }

    template <typename ResultType>
    std::future<ResultType> DelayedTaskActuator<ResultType>::getTaskResult(int32_t taskId)
    {
        auto state = state_;
        std::unique_lock lock(state->mutex);

        // Wait until the task result is available or task has completed execution
        state->cv.wait(lock, [state, taskId]
        {
            return state->results.contains(taskId) || !isTaskPendingLocked(state->pendingTasks, taskId);
        });

        if (!state->alive)
        {
            throw std::runtime_error("DelayedTaskActuator::getTaskResult: Actuator is destroyed");
        }

        auto it = state->results.find(taskId);
        if (it != state->results.end())
        {
            std::future<ResultType> result = std::move(it->second);
            state->results.erase(it);
            return result;
        }
        throw std::runtime_error("DelayedTaskActuator::getTaskResult: Task result not available, possibly already retrieved");
    }

    template <typename ResultType>
    bool DelayedTaskActuator<ResultType>::isTaskPendingLocked(const std::unordered_map<int32_t, std::shared_ptr<bool>>& pendingTasks, const int32_t taskId)
    {
        const auto it = pendingTasks.find(taskId);
        if (it != pendingTasks.end())
        {
            return *it->second;
        }
        return false;
    }

    template <typename ResultType>
    bool DelayedTaskActuator<ResultType>::isTaskPending(const int32_t taskId) const
    {
        std::lock_guard lock(state_->mutex);
        return isTaskPendingLocked(state_->pendingTasks, taskId);
    }

    template <typename ResultType>
    bool DelayedTaskActuator<ResultType>::cancelTask(int32_t taskId)
    {
        std::lock_guard lock(state_->mutex);
        const auto pendingIt = state_->pendingTasks.find(taskId);
        if (pendingIt == state_->pendingTasks.end())
        {
            return false;
        }

        auto it = state_->results.find(taskId);
        if (it != state_->results.end())
        {
            state_->results.erase(it);
            state_->pendingTasks.erase(pendingIt);
            return true;
        }
        return false;
    }
}
